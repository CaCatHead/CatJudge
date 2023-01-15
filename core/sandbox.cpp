#include <pwd.h>
#include <cerrno>
#include <unistd.h>

#include <sys/reg.h>
#include <sys/time.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "conf.h"
#include "context.h"
#include "sandbox.h"
#include "rf_table.h"

extern int errno;

using CONF::EXIT;
using CONF::Verdict;
using CONF::Language;

/**
 * 超时的回调函数
 */
static void timeout(int sig) {
  if (sig == SIGALRM) {
    FM_LOG_TRACE("Receive SIGALRM, exit process.");
    exit(EXIT::TIMEOUT);
  }
}

/**
 * 设置时间限制
 */
static int malarm(int which, int milliseconds) {
  struct itimerval t {};
  t.it_value.tv_sec = milliseconds / 1000;
  t.it_value.tv_usec = milliseconds % 1000 * 1000; //微秒
  t.it_interval.tv_sec = 0;
  t.it_interval.tv_usec = 0;
  return setitimer(which, &t, nullptr);
}

/*
 * 输入输出重定向
 */
static void redirect_io(std::string in, std::string out, std::string err) {
  FM_LOG_DEBUG("Start redirecting IO");

  // 输出文件权限控制
  chmod(out.c_str(), S_IRUSR | S_IWUSR);
  chmod(err.c_str(), S_IRUSR | S_IWUSR);

  stdin = freopen(in.c_str(), "r", stdin);
  stdout = freopen(out.c_str(), "w", stdout);
  stderr = freopen(err.c_str(), "w", stderr);

  if (stdin == nullptr || stdout == nullptr) {
    FM_LOG_WARNING("It occurred an error when freopen: stdin(%p) stdout(%p)", stdin, stdout);
    exit(EXIT::PRE_JUDGE);
  }

  FM_LOG_DEBUG("Redirecting IO is OK");
}

/*
 * 输出重定向
 */
static void redirect_io(std::string out, std::string err) {
  FM_LOG_DEBUG("Start redirecting stdout and stderr");

  // 输出文件权限控制
  chmod(out.c_str(), S_IRUSR | S_IWUSR);
  chmod(err.c_str(), S_IRUSR | S_IWUSR);

  stdout = freopen(out.c_str(), "w", stdout);
  stderr = freopen(err.c_str(), "w", stderr);

  if (stdout == nullptr || stderr == nullptr) {
    FM_LOG_WARNING("It occurred an error when freopen: stdin(%p) stdout(%p)", stdin, stdout);
    exit(EXIT::PRE_JUDGE);
  }

  FM_LOG_DEBUG("Redirecting IO is OK");
}

static const size_t MAX_BUFFER_SIZE = 1024;
static char buffer[MAX_BUFFER_SIZE + 4];

static std::string read_text(std::string path) {
  FILE *fp = fopen(path.c_str(), "r");
  if (!fp) {
    return std::string();
  }
  size_t ok = fread(&buffer, MAX_BUFFER_SIZE, 1, fp);
  return std::string(buffer);
}

/*
 * 程序运行的限制
 * CPU时间、堆栈、输出文件大小等
 */
static void set_limit(Context *ctx) {
  rlimit lim {};

  lim.rlim_max = (ctx->time_limit - ctx->result->time + 999) / 1000 + 1;//硬限制
  lim.rlim_cur = lim.rlim_max; //软限制
  if (setrlimit(RLIMIT_CPU, &lim) < 0) {
    FM_LOG_WARNING("error setrlimit for RLIMIT_CPU");
    exit(EXIT::SET_LIMIT);
  }

  // 内存不能在此做限制
  // 原因忘了，反正是linux的内存分配机制的问题
  // 所以得在运行时不断累计内存使用量来限制


  // 堆栈空间限制
  getrlimit(RLIMIT_STACK, &lim);

  // 比内存限制多加了 4MB
  int rlim = ctx->memory_limit * CONF::KILO + 4 * CONF::MEGA;
  FM_LOG_DEBUG("stack limit size is %d KB", lim.rlim_max);

  if (0 < lim.rlim_max && lim.rlim_max <= rlim) {
    FM_LOG_WARNING("cannot set stack size to higher (%d <= %d)", lim.rlim_max, rlim);
  } else {
    FM_LOG_DEBUG("setrlimit for RLIMIT_STACK is %d KB", rlim);

    lim.rlim_max = rlim;
    lim.rlim_cur = rlim;

    if (setrlimit(RLIMIT_STACK, &lim) < 0) {
      FM_LOG_WARNING("error setrlimit for RLIMIT_STACK");
      exit(EXIT::SET_LIMIT);
    }
  }

//#ifndef __DEBUG__
//  // Release build: 关闭 log, 防止 log 造成 OLE
//  log_close();
//#endif

  // 输出文件大小限制
  lim.rlim_max = ctx->output_limit * CONF::KILO;
  lim.rlim_cur = lim.rlim_max;
  if (setrlimit(RLIMIT_FSIZE, &lim) < 0) {
    perror("setrlimit RLIMIT_FSIZE failed\n");
    exit(EXIT::SET_LIMIT);
  }
}

/*
 * 安全性控制
 * chroot 限制程序只能在某目录下操作，无法影响到外界
 * setuid 使其只拥有 nobody 的最低系统权限
 */
static void security_control(Context *ctx) {
  struct passwd *nobody = getpwnam("nobody");

  if (nobody == nullptr) {
    FM_LOG_WARNING("Well, where is nobody? I cannot live without him. %d: %s", errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  // chdir
  if (EXIT_SUCCESS != chdir(ctx->run_dir)) {
    FM_LOG_WARNING("chdir(%s) failed, %d: %s", ctx->run_dir, errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  static char cwd[1024];
  char *tmp = getcwd(cwd, 1024);
  if (tmp == nullptr) {
    FM_LOG_WARNING("Oh, where i am now? I cannot getcwd. %d: %s", errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  // chroot
  // Java 比较特殊，一旦 chroot 或 setuid，那么 JVM 就跑不起来了
  if (ctx->language != Language::JAVA) {
    if (EXIT_SUCCESS != chroot(cwd)) {
      FM_LOG_WARNING("chroot(%s) failed. %d: %s", cwd, errno, strerror(errno));
      exit(EXIT::SET_SECURITY);
    }
  }

  // setuid is ok for Java
  if (EXIT_SUCCESS != setuid(nobody->pw_uid)) {
    FM_LOG_WARNING("setuid(%d) failed. %d: %s", nobody->pw_uid, errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }
}

/*
 * 对 SpecialJudge 程序的安全性控制, 毕竟不是自己写的代码，得防着点
 */
static void security_control_checker(Context *ctx) {
  struct passwd *nobody = getpwnam("nobody");
  if (nobody == nullptr) {
    FM_LOG_WARNING("Well, where is nobody? I cannot live without him. %d: %s", errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  if (EXIT_SUCCESS != chdir(ctx->run_dir)) {
    FM_LOG_WARNING("chdir(%s) failed, %d: %s", ctx->run_dir, errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  static char cwd[1024];
  char *tmp = getcwd(cwd, 1024);
  if (tmp == nullptr) {
    FM_LOG_WARNING("Oh, where i am now? I cannot getcwd. %d: %s", errno, strerror(errno));
    exit(EXIT::SET_SECURITY);
  }

  //if (PROBLEM::spj_lang != JUDGE_CONF::LANG_JAVA) {
  //    if (EXIT_SUCCESS != chroot(cwd)) {
  //        FM_LOG_WARNING("chroot(%s) failed. %d: %s", cwd, errno, strerror(errno));
  //        exit(JUDGE_CONF::EXIT_SET_SECURITY);
  //    }
  //}

  //if (EXIT_SUCCESS != setuid(nobody->pw_uid)) {
  //    FM_LOG_WARNING("setuid(%d) failed. %d: %s", nobody->pw_uid, errno, strerror(errno));
  //    exit(JUDGE_CONF::EXIT_SET_SECURITY);
  //}
}


/*
 * 执行用户提交的程序
 */
static Result *run(Context *ctx) {
  auto *result = new Result();
  ctx->result = result;

  struct rusage rused {};

  pid_t executive = fork();

  if (executive < 0) {
    exit(EXIT::PRE_JUDGE);
  } else if (executive == 0) {
    // 子进程: 用户程序
    FM_LOG_DEBUG("Fork child process");

    redirect_io(ctx->input_file(), ctx->output_file(), ctx->error_file());

    security_control(ctx);

    // 定时器设置两倍时间限制
    int real_time_limit = 2 * ctx->time_limit;
    if (EXIT_SUCCESS != malarm(ITIMER_REAL, real_time_limit)) {
      exit(EXIT::PRE_JUDGE);
    }

    set_limit(ctx);

    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) < 0) {
      exit(EXIT::PRE_JUDGE_PTRACE);
    }

    FM_LOG_TRACE("Start running user submission");
    if (ctx->language == Language::C || ctx->language == Language::CPP) {
      int err = execl("./a.out", "a.out", NULL);
      if (err == -1) {
        FM_LOG_FATAL("Execl error: %d", errno);
      }
    } else {
      int err = execlp("java", "java", "Main", NULL);
      if (err == -1) {
        FM_LOG_FATAL("Execlp error: %d", errno);
      }
    }

    // 走到这了说明出错了
    exit(EXIT::PRE_JUDGE_EXECLP);
  } else {
    // 记录首次启动时使用的内存？
    getrusage(RUSAGE_SELF, &rused);
    FM_LOG_DEBUG("Init memory: %d KB", rused.ru_maxrss);
    long int init_mem = rused.ru_maxrss;
    if (init_mem < 0) {
      init_mem = 0;
    }

    // 父进程
    int status = 0;  // 子进程状态
    int syscall_id = 0; // 系统调用号

    struct user_regs_struct regs {}; // 寄存器

    init_RF_table(ctx->language); // 初始化系统调用表

    while (true) {// 循环监控子进程
      if (wait4(executive, &status, 0, &rused) < 0) {
        FM_LOG_WARNING("wait4 failed.");
        exit(EXIT::JUDGE);
      }

      // MLE, using ru_maxrss (since Linux 2.6.32)
      result->memory = std::max((long int) result->memory, rused.ru_maxrss - init_mem);
      if (result->verdict == Verdict::SE && result->memory > ctx->memory_limit) {
        result->verdict = Verdict::MLE;
        FM_LOG_TRACE("Memory Limit Exceeded (%d KB)", result->memory);
        ptrace(PTRACE_KILL, executive, NULL, NULL);
        break;
      }

      // 自行退出
      if (WIFEXITED(status)) {
        if (ctx->language != Language::JAVA ||
            WEXITSTATUS(status) == EXIT_SUCCESS) {
          FM_LOG_TRACE("OK, normal quit. All is good.");
        } else {
          FM_LOG_WARNING("Oh, some error occurred. Abnormal quit.");
          result->verdict = Verdict::RE;
        }
        break;
      }

      // 被信号终止掉了
      // 要过滤掉 SIGTRAP 信号
      if (WIFSIGNALED(status) || (WIFSTOPPED(status) && WSTOPSIG(status) != SIGTRAP)) {
        int sig;
        if (WIFSIGNALED(status)) {
          sig = WTERMSIG(status);
          FM_LOG_WARNING("Child is signaled by %d : %s", sig, strsignal(sig));
        } else {
          sig = WSTOPSIG(status);
          FM_LOG_WARNING("Child is stopped by %d : %s", sig, strsignal(sig));
        }

        switch (sig) {
          // TLE
          case SIGALRM:
          case SIGXCPU:
          case SIGVTALRM:
          case SIGKILL:
            FM_LOG_TRACE("Time Limit Exceeded");
            result->verdict = Verdict::TLE;
            break;
          case SIGXFSZ:
            FM_LOG_TRACE("File Limit Exceeded");
            result->verdict = Verdict::OLE;
            break;
          case SIGSEGV:
          case SIGFPE:
          case SIGBUS:
          case SIGABRT:
            FM_LOG_TRACE("Runtime Error");
            result->verdict = Verdict::RE;
            break;
          default:
            FM_LOG_TRACE("Unknown Error");
            result->verdict = Verdict::RE;
            break;
        }

        ptrace(PTRACE_KILL, executive, NULL, NULL);
        break;
      }

      // 获得子进程的寄存器，目的是为了获知其系统调用
      if (ptrace(PTRACE_GETREGS, executive, NULL, &regs) < 0) {
        FM_LOG_WARNING("ptrace PTRACE_GETREGS failed");
        exit(EXIT::JUDGE);
      }

#ifdef __i386__
      syscall_id = regs.orig_eax;
#else
      syscall_id = regs.orig_rax;
#endif
      // 检查系统调用是否合法
      if (syscall_id > 0 &&
          !is_valid_syscall(ctx->language, syscall_id, executive, regs)) {
        FM_LOG_WARNING("Restricted system call: %d.", syscall_id);
        if (syscall_id == SYS_rt_sigprocmask) {
          FM_LOG_WARNING("The glibc failed.");
        } else {
          FM_LOG_WARNING("In restricted function table.");
        }
        result->verdict = Verdict::RE;
        ptrace(PTRACE_KILL, executive, NULL, NULL);
        break;
      }

      if (ptrace(PTRACE_SYSCALL, executive, NULL, NULL) < 0) {
        FM_LOG_WARNING("ptrace PTRACE_SYSCALL failed.");
        exit(EXIT::JUDGE);
      }
    }
  }

  FM_LOG_TRACE("Running user submission is OK");

  result->time = 0;
  result->time += (rused.ru_utime.tv_sec * 1000 + rused.ru_utime.tv_usec / 1000);
  result->time += (rused.ru_stime.tv_sec * 1000 + rused.ru_stime.tv_usec / 1000);

  if (result->verdict == Verdict::SE) {
    if (result->time > ctx->time_limit) {
      result->verdict = Verdict::TLE;
    } else if (result->memory > ctx->memory_limit) {
      result->verdict = Verdict::MLE;
    }
  }

  return result;
}

static Result *check(Context *ctx) {
  struct rusage rused {};
  getrusage(RUSAGE_SELF, &rused);
  long int init_mem = rused.ru_maxrss;
  if (init_mem < 0) {
    init_mem = 0;
  }
  FM_LOG_DEBUG("Checker init memory: %d KB", init_mem);

  pid_t spj_pid = fork();
  int status = 0;

  if (spj_pid < 0) {
    FM_LOG_WARNING("Fork special judge failed.");
    exit(EXIT::COMPARE_SPJ);
  } else if (spj_pid == 0) {
    FM_LOG_TRACE("Start checking.");

#ifndef __DEBUG__
    stdout = freopen(ctx->checker_output_file().c_str(), "w", stdout);
    stderr = freopen(ctx->checker_error_file().c_str(), "w", stderr);

    if (stdout == NULL || stderr == NULL) {
      FM_LOG_WARNING("It occurred an error when freopen: stdin(%p) stdout(%p)", stdin, stdout);
      exit(EXIT::COMPARE_SPJ);
    }
#endif

    // SPJ 时间限制
    if (EXIT_SUCCESS != malarm(ITIMER_REAL, ctx->time_limit * 2)) {
      FM_LOG_WARNING("Set time limit for spj failed.");
      exit(EXIT::COMPARE_SPJ);
    }

    redirect_io(ctx->checker_output_file(), ctx->checker_error_file());

    security_control_checker(ctx);

    std::string checker_path = std::string(CHECKER_ROOT) + "/" + ctx->checker_name();
    // Only support executing binary
    int err = execl(
        checker_path.c_str(),
        ctx->checker_name().c_str(),
        ctx->input_file().c_str(),
        ctx->output_file().c_str(),
        ctx->answer_file().c_str(), NULL);
    if (err == -1) {
      FM_LOG_FATAL("Execl checker error: %d", errno);
    }

    exit(EXIT::COMPARE_SPJ_FORK);
  } else {
    if (wait4(spj_pid, &status, 0, &rused) < 0) {
      FM_LOG_WARNING("spj wait4 failed.");
      exit(EXIT::COMPARE_SPJ);
    }

    ctx->result->checker_memory = std::max((long int) ctx->result->checker_memory, rused.ru_maxrss - init_mem);

    FM_LOG_DEBUG("Checker return code: %d", status);

    if (WIFEXITED(status)) {
      int return_code = WEXITSTATUS(status);
      if (return_code == EXIT_SUCCESS) {
        FM_LOG_TRACE("Checker normally quit.");
        ctx->result->verdict = Verdict::AC;
      } else if (return_code == 3) {
        FM_LOG_TRACE("Checker failed.");
        ctx->result->verdict = Verdict::SE;
      } else {
        ctx->result->verdict = Verdict::WA;
      }
    } else if (WIFSIGNALED(status) && WTERMSIG(status) == SIGALRM) {
      ctx->result->verdict = Verdict::SE;
      FM_LOG_WARNING("Well, the special judge program consume too much time.");
    } else {
      ctx->result->verdict = Verdict::SE;
      FM_LOG_WARNING("Actually, I do not know why the special judge program dead.");
    }
  }

  ctx->result->checker_time = 0;
  ctx->result->checker_time += (rused.ru_utime.tv_sec * 1000 + rused.ru_utime.tv_usec / 1000);
  ctx->result->checker_time += (rused.ru_stime.tv_sec * 1000 + rused.ru_stime.tv_usec / 1000);

  ctx->result->checker_message = read_text(ctx->checker_error_file());

  return ctx->result;
}

Result *judge(Context *ctx) {
  // 定时器设置两倍时间限制，加上评测时限
  if (EXIT_SUCCESS != malarm(ITIMER_REAL, 2 * ctx->time_limit + CONF::JUDGE_TIME_LIMIT)) {
    FM_LOG_WARNING("Set the alarm for this judge program failed, %d: %s", errno, strerror(errno));
    exit(EXIT::VERY_FIRST);
  }

  signal(SIGALRM, timeout);

  run(ctx);

  if (ctx->result->verdict == Verdict::SE) {
    return check(ctx);
  } else {
    return ctx->result;
  }
}
