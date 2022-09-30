#include <errno.h>
#include <unistd.h>

#include <sys/reg.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "conf.h"
#include "context.h"
#include "sandbox.h"
#include "rf_table.h"

extern int errno;

/**
 * 超时的回调函数
 */
static void timeout(int sig) {
  if (sig == SIGALRM) {
    exit(CONF::EXIT::TIMEOUT);
  }
}

/**
 * 设置时间限制
 */
static int malarm(int which, int milliseconds) {
  struct itimerval t;
  t.it_value.tv_sec = milliseconds / 1000;
  t.it_value.tv_usec = milliseconds % 1000 * 1000; //微秒
  t.it_interval.tv_sec = 0;
  t.it_interval.tv_usec = 0;
  return setitimer(which, &t, NULL);
}

/*
 * 输入输出重定向
 */
static void redirect_io() {
  FM_LOG_TRACE("Start to redirect the IO.");

  stdin = freopen(PROBLEM::input_file.c_str(), "r", stdin);
  stdout = freopen(PROBLEM::exec_output.c_str(), "w", stdout);
  //stderr = freopen("/dev/null", "w", stderr);

  if (stdin == NULL || stdout == NULL) {
    FM_LOG_WARNING("It occur a error when freopen: stdin(%p) stdout(%p)", stdin, stdout);
    exit(CONF::EXIT::PRE_JUDGE);
  }

  FM_LOG_TRACE("redirect io is OK.");
}

/*
 * 程序运行的限制
 * CPU时间、堆栈、输出文件大小等
 */
static void set_limit() {
  rlimit lim;

  lim.rlim_max = (PROBLEM::time_limit - PROBLEM::time_usage + 999) / 1000 + 1;//硬限制
  lim.rlim_cur = lim.rlim_max; //软限制
  if (setrlimit(RLIMIT_CPU, &lim) < 0) {
    FM_LOG_WARNING("error setrlimit for RLIMIT_CPU");
    exit(JUDGE_CONF::EXIT_SET_LIMIT);
  }

  //内存不能在此做限制
  //原因忘了，反正是linux的内存分配机制的问题
  //所以得在运行时不断累计内存使用量来限制


  //堆栈空间限制
  getrlimit(RLIMIT_STACK, &lim);

  int rlim = JUDGE_CONF::STACK_SIZE_LIMIT * JUDGE_CONF::KILO;
  if (lim.rlim_max <= rlim) {
    FM_LOG_WARNING("cannot set stack size to higher(%d <= %d)", lim.rlim_max, rlim);
  } else {
    lim.rlim_max = rlim;
    lim.rlim_cur = rlim;

    if (setrlimit(RLIMIT_STACK, &lim) < 0) {
      FM_LOG_WARNING("error setrlimit for RLIMIT_STACK");
      exit(JUDGE_CONF::EXIT_SET_LIMIT);
    }
  }

  log_close(); //关闭log，防止log造成OLE

  //输出文件大小限制
  lim.rlim_max = PROBLEM::output_limit * JUDGE_CONF::KILO;
  lim.rlim_cur = lim.rlim_max;
  if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
    perror("setrlimit RLIMIT_FSIZE failed\n");
    exit(JUDGE_CONF::EXIT_SET_LIMIT);
  }
}


/*
 * 执行用户提交的程序
 */
//static Result* run(Context* ctx) {
//  struct Result* result = new Result();
//
//  struct rusage rused;
//
//  pid_t executive = fork();
//
//  if (executive < 0) {
//    exit(CONF::EXIT::PRE_JUDGE);
//  } else if (executive == 0) {
//    // 子进程，用户程序
//    FM_LOG_TRACE("Start Judging.");
//
//    redirect_io();
//
//    security_control();
//
//    int real_time_limit = PROBLEM::time_limit;
//    if (EXIT_SUCCESS != malarm(ITIMER_REAL, real_time_limit)) {
//      exit(JUDGE_CONF::EXIT_PRE_JUDGE);
//    }
//
//    set_limit();
//
//    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
//      exit(CONF::EXIT::PRE_JUDGE_PTRACE);
//    }
//
//    if (ctx->language == CONF::Language::C || ctx->language == CONF::Language::CPP) {
//      execl("./a.out", "a.out", NULL);
//    } else {
//      execlp("java", "java", "Main", NULL);
//    }
//
//    // 走到这了说明出错了
//    exit(CONF::EXIT::PRE_JUDGE_EXECLP);
//  } else {
//    // 父进程
//    int status = 0;  //子进程状态
//    int syscall_id = 0; //系统调用号
//
//    struct user_regs_struct regs; //寄存器
//
//    init_RF_table(ctx->language); //初始化系统调用表
//
//    while (true) {//循环监控子进程
//      if (wait4(executive, &status, 0, &rused) < 0) {
//        FM_LOG_WARNING("wait4 failed.");
//        exit(JUDGE_CONF::EXIT_JUDGE);
//      }
//
//      // 自行退出
//      if (WIFEXITED(status)) {
//        if (ctx->language != CONF::Language::JAVA ||
//            WEXITSTATUS(status) == EXIT_SUCCESS) {
//          FM_LOG_TRACE("OK, normal quit. All is good.");
//
//          //PROBLEM::result = JUDGE_CONF::PROCEED;
//        } else {
//          FM_LOG_WARNING("oh, some error occurred. Abnormal quit.");
//          PROBLEM::result = JUDGE_CONF::RE;
//        }
//        break;
//      }
//
//      //被信号终止掉了
//      if (WIFSIGNALED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) { //要过滤掉SIGTRAP信号
//        int sig = 0;
//        if (WIFSIGNALED(status)) {
//          sig = WTERMSIG(status);
//          FM_LOG_WARNING("child signaled by %d : %s", sig, strsignal(sig));
//        } else {
//          sig = WSTOPSIG(status);
//          FM_LOG_WARNING("child stop by %d : %s\n", sig, strsignal(sig));
//        }
//
//        switch (sig) {
//          //TLE
//          case SIGALRM:
//          case SIGXCPU:
//          case SIGVTALRM:
//          case SIGKILL:
//            FM_LOG_TRACE("Well, Time Limit Exceeded");
//            PROBLEM::time_usage = 0;
//            PROBLEM::memory_usage = 0;
//            PROBLEM::result = JUDGE_CONF::TLE;
//            break;
//          case SIGXFSZ:
//            FM_LOG_TRACE("File Limit Exceeded");
//            PROBLEM::time_usage = 0;
//            PROBLEM::memory_usage = 0;
//            PROBLEM::result = JUDGE_CONF::OLE;
//            break;
//          case SIGSEGV:
//          case SIGFPE:
//          case SIGBUS:
//          case SIGABRT:
//            //FM_LOG_TRACE("RE了");
//            PROBLEM::time_usage = 0;
//            PROBLEM::memory_usage = 0;
//            PROBLEM::result = JUDGE_CONF::RE;
//            break;
//          default:
//            //FM_LOG_TRACE("不知道哪儿跪了");
//            PROBLEM::time_usage = 0;
//            PROBLEM::memory_usage = 0;
//            PROBLEM::result = JUDGE_CONF::RE;
//            break;
//        }
//
//        ptrace(PTRACE_KILL, executive, NULL, NULL);
//        break;
//      }
//
//      //MLE
//      PROBLEM::memory_usage = std::max((long int) PROBLEM::memory_usage,
//                                       rused.ru_minflt * (getpagesize() / JUDGE_CONF::KILO));
//
//      if (PROBLEM::memory_usage > PROBLEM::memory_limit) {
//        PROBLEM::time_usage = 0;
//        PROBLEM::memory_usage = 0;
//        PROBLEM::result = JUDGE_CONF::MLE;
//        FM_LOG_TRACE("Well, Memory Limit Exceeded.");
//        ptrace(PTRACE_KILL, executive, NULL, NULL);
//        break;
//      }
//
//      //获得子进程的寄存器，目的是为了获知其系统调用
//      if (ptrace(PTRACE_GETREGS, executive, NULL, &regs) < 0) {
//        FM_LOG_WARNING("ptrace PTRACE_GETREGS failed");
//        exit(CONF::EXIT::JUDGE);
//      }
//
//#ifdef __i386__
//      syscall_id = regs.orig_eax;
//#else
//      syscall_id = regs.orig_rax;
//#endif
//      //检查系统调用是否合法
//      if (syscall_id > 0 &&
//          !is_valid_syscall(PROBLEM::lang, syscall_id, executive, regs)) {
//        FM_LOG_WARNING("restricted function %d\n", syscall_id);
//        if (syscall_id == SYS_rt_sigprocmask) {
//          FM_LOG_WARNING("The glibc failed.");
//        } else {
//          //FM_LOG_WARNING("%d\n", SYS_write);
//          FM_LOG_WARNING("restricted function table");
//        }
//        PROBLEM::result = JUDGE_CONF::RE;
//        ptrace(PTRACE_KILL, executive, NULL, NULL);
//        break;
//      }
//
//      if (ptrace(PTRACE_SYSCALL, executive, NULL, NULL) < 0) {
//        FM_LOG_WARNING("ptrace PTRACE_SYSCALL failed.");
//        exit(CONF::EXIT::JUDGE);
//      }
//    }
//  }
//
//  //这儿关于time_usage和memory_usage计算的有点混乱
//  //主要是为了减轻web的任务
//  //只要不是AC，就把time_usage和memory_usage归0
//  if (PROBLEM::result == JUDGE_CONF::SE) {
//    PROBLEM::time_usage += (rused.ru_utime.tv_sec * 1000 +
//                            rused.ru_utime.tv_usec / 1000);
//    PROBLEM::time_usage += (rused.ru_stime.tv_sec * 1000 +
//                            rused.ru_stime.tv_usec / 1000);
//  }
//
//  return result;
//}

Result *judge(Context *ctx) {
  if (EXIT_SUCCESS != malarm(ITIMER_REAL, ctx->time_limit + CONF::JUDGE_TIME_LIMIT)) {
    FM_LOG_WARNING("Set the alarm for this judge program failed, %d: %s", errno, strerror(errno));
    exit(CONF::EXIT::VERY_FIRST);
  }

  signal(SIGALRM, timeout);

//  run(ctx);

  return nullptr;
}
