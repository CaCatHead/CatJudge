#ifndef __RF_TABLE__
#define __RF_TABLE__

#include <string.h>
#include <sys/syscall.h>

#include "conf.h"
#include "logger.h"

/*
 * RF_table 每个值对应的是该syscall可被调用的次数
 *    取值有3种:
 *      正值: 表示可被调用的次数, 每次调用后会减一(比如fork)
 *      零值: 表示禁止调用(比如open)
 *      负值: 表示不限制该syscall(比如write)
 *
 * RF_table的初始化由init_RF_table函数完成
 */
short RF_table[1024] = {0};

/*
 * RF_* 数组对是用于初始化RF_table的数据来源,
 * 每两个数字为一组k/v: syscall_id:次数
 *   次数 < 0 表示不限制
 *   次数 > 0 表示可调用次数, 运行时每调用一次减一
 *   次数 = 0 (运行时达到) 不再允许调用该syscall
 * 最后一个syscall_id存放的是非正值，表示结束
 * 未指定的syscall_id的次数将被自动初始化为0
 */
#if __WORDSIZE == 32    //x86_32
//c or c++
int RF_C[512] =
{
    SYS_access,         -1,
    SYS_brk,            -1,
    SYS_close,          -1,
    SYS_execve,         1,
    SYS_exit_group,     -1,
    SYS_fstat64,        -1,
    SYS_futex,          -1,
    SYS_gettimeofday,   -1,
    SYS_mmap2,          -1,
    SYS_mremap,         -1,
    SYS_mprotect,       -1,
    SYS_munmap,         -1,
    SYS_lseek,          -1,
    SYS_read,           -1,
    SYS_set_thread_area,-1,
    SYS_uname,          -1,
    SYS_write,          -1,
    SYS_writev,         -1,
  201,				-1,
    -1
};

int RF_CPP[512] =
{
    SYS_access,         -1,
    SYS_brk,            -1,
    SYS_close,          -1,
    SYS_execve,         1,
    SYS_exit_group,     -1,
    SYS_fstat64,        -1,
    SYS_futex,          -1,
    SYS_gettimeofday,   -1,
    SYS_mmap2,          -1,
    SYS_mremap,         -1,
    SYS_mprotect,       -1,
    SYS_munmap,         -1,
    SYS_lseek,          -1,
    SYS_read,           -1,
    SYS_set_thread_area,-1,
    SYS_uname,          -1,
    SYS_write,          -1,
    SYS_writev,         -1,
  201,				-1,
    -1
};

//Pascal
int RF_PASCAL[512] =
{
    SYS_close,          -1,
    SYS_execve,         1,
    SYS_exit_group,     -1,
    SYS_futex,          -1,
    SYS_gettimeofday,   -1,
    SYS_ioctl,          -1,
    SYS_mmap,           -1,
    SYS_mremap,         -1,
    SYS_munmap,         -1,
    SYS_lseek,          -1,
    SYS_read,           -1,
    SYS_readlink,       -1,
    SYS_rt_sigaction,   -1,
    SYS_ugetrlimit,     -1,
    SYS_uname,          -1,
    SYS_write,          -1,
    SYS_writev,         -1,
    -1
};

//Java (TODO暂未测试)
int RF_JAVA[512] =
{
    SYS_access,         -1,
    SYS_brk,            -1,
    SYS_clone,          -1,
    SYS_close,          -1,
    SYS_execve,         -1,
    SYS_exit,           -1,
    SYS_exit_group,     -1,
    SYS_fcntl64,        -1,
    SYS_fstat64,        -1,
    SYS_futex,          -1,
    SYS_getdents64,     -1,
    SYS_getegid32,      -1,
    SYS_geteuid32,      -1,
    SYS_getgid32,       -1,
    SYS_getrlimit,      -1,
    SYS_gettimeofday,   -1,
    SYS_getuid32,       -1,
    SYS_mmap,           -1,
    SYS_mmap2,          -1,
    SYS_mremap,         -1,
    SYS_mprotect,       -1,
    SYS_munmap,         -1,
    SYS_lseek,          -1,
    SYS_open,           -1,
    SYS_read,           -1,
    SYS_readlink,       -1,
    SYS_rt_sigaction,   -1,
    SYS_rt_sigprocmask, -1,
    SYS_set_robust_list,-1,
    SYS_set_thread_area,-1,
    SYS_set_tid_address,-1,
    SYS_sigprocmask,    -1,
    SYS_stat64,         -1,
    SYS_ugetrlimit,     -1,
    SYS_uname,          -1,
    -1
};

#elif defined __x86_64__
int RF_C[512] =
    {
        SYS_access, -1,
        SYS_arch_prctl, -1,
        SYS_brk, -1,
        SYS_close, -1,
        SYS_execve, 1,
        SYS_exit_group, -1,
        SYS_fstat, -1,
        SYS_futex, -1,
        SYS_gettimeofday, -1,
        SYS_mmap, -1,
        SYS_mremap, -1,
        SYS_mprotect, -1,
        SYS_munmap, -1,
        SYS_lseek, -1,
        SYS_read, -1,
        SYS_set_thread_area, -1,
        SYS_uname, -1,
        SYS_write, -1,
        SYS_writev, -1,
        SYS_time, -1,
        SYS_readlink, -1,
        -1
    };

int RF_CPP[512] =
    {
        SYS_access, -1,
        SYS_arch_prctl, -1,
        SYS_brk, -1,
        SYS_close, -1,
        SYS_execve, 1,
        SYS_exit_group, -1,
        SYS_fstat, -1,
        SYS_futex, -1,
        SYS_gettimeofday, -1,
        SYS_mmap, -1,
        SYS_mremap, -1,
        SYS_mprotect, -1,
        SYS_munmap, -1,
        SYS_lseek, -1,
        SYS_read, -1,
        SYS_set_thread_area, -1,
        SYS_uname, -1,
        SYS_write, -1,
        SYS_writev, -1,
        SYS_time, -1,
        SYS_readlink, -1, //原本ubuntu 12.04不需要这一条
        -1
    };

int RF_PASCAL[512] =
    {
        SYS_close, -1,
        SYS_execve, 1,
        SYS_exit_group, -1,
        SYS_futex, -1,
        SYS_getrlimit, -1,
        SYS_gettimeofday, -1,
        SYS_ioctl, -1,
        SYS_mmap, -1,
        SYS_mremap, -1,
        SYS_munmap, -1,
        SYS_lseek, -1,
        SYS_read, -1,
        SYS_readlink, -1,
        SYS_rt_sigaction, -1,
        SYS_uname, -1,
        SYS_write, -1,
        SYS_writev, -1,
        SYS_time, -1,
        SYS_readlink, -1,
        -1
    };

int RF_JAVA[512] =
    {
        SYS_access, -1,
        SYS_arch_prctl, -1,
        SYS_brk, -1,
        SYS_clone, -1,
        SYS_close, -1,
        SYS_execve, -1,
        SYS_exit_group, -1,
        SYS_fstat, -1,
        SYS_futex, -1,
        SYS_getegid, -1,
        SYS_geteuid, -1,
        SYS_getgid, -1,
        SYS_getrlimit, -1,
        SYS_gettimeofday, -1,
        SYS_getuid, -1,
        SYS_mmap, -1,
        SYS_mremap, -1,
        SYS_mprotect, -1,
        SYS_munmap, -1,
        SYS_lseek, -1,
        SYS_open, -1,
        SYS_read, -1,
        SYS_readlink, -1,
        SYS_rt_sigaction, -1,
        SYS_rt_sigprocmask, -1,
        SYS_set_robust_list, -1,
        SYS_set_tid_address, -1,
        SYS_stat, -1,
        SYS_uname, -1,
        SYS_write, -1,
        SYS_writev, -1,
        SYS_time, -1,
        SYS_readlink, -1,
        -1
    };
#endif

//根据 RF_* 数组来初始化 RF_table
void init_RF_table(int lang) {
  int *p = NULL;
  switch (lang) {
    case CONF::Language::C:
      p = RF_C;
      break;
    case CONF::Language::CPP:
      p = RF_CPP;
      break;
    case CONF::Language::JAVA:
      p = RF_JAVA;
      break;
    default:
      FM_LOG_WARNING("Unknown language: %d", lang);
      break;
  }
  memset(RF_table, 0, sizeof(RF_table));
  for (int i = 0; p[i] >= 0; i += 2) {
    RF_table[p[i]] = p[i + 1];
  }
}

// 系统调用在进和出的时候都会暂停, 把控制权交给judge
static bool in_syscall = true;

bool is_valid_syscall(int lang, int syscall_id, pid_t child, user_regs_struct regs) {
  in_syscall = !in_syscall;
  // FM_LOG_DEBUG("syscall: %d, %s, count: %d", syscall_id, in_syscall?"in":"out", RF_table[syscall_id]);
  if (RF_table[syscall_id] == 0) {
    // 如果 RF_table 中对应的 syscall_id 可以被调用的次数为 0, 则为 RF
    long addr;
    if (syscall_id == SYS_open) {
#if __WORDSIZE == 32
      addr = regs.ebx;
#else
      addr = regs.rdi;
#endif

      const int LONGSIZE = sizeof(long);
      
      union u {
        unsigned long val;
        char chars[LONGSIZE];
      } data;
      
      unsigned long i = 0, j = 0, k = 0;
      static char filename[1024];
      while (true) {
        data.val = ptrace(PTRACE_PEEKDATA, child, addr + i, NULL);
        i += LONGSIZE;
        for (j = 0; j < LONGSIZE && data.chars[j] > 0 && k < 256; j++) {
          filename[k++] = data.chars[j];
        }
        if (j < LONGSIZE && data.chars[j] == 0)
          break;
      }
      filename[k] = 0;
      
      // FM_LOG_TRACE("syscall open: filename: %s", filename);
      if (strstr(filename, "..") != NULL) {
        return false;
      }
      if (strstr(filename, "/proc/") == filename) {
        return true;
      }
      if (strstr(filename, "/dev/tty") == filename) {
        // TODO: ?
        // PROBLEM::result = Verdict::RE;
        exit(CONF::EXIT::OK);
      }
    }
    return false;
  } else if (RF_table[syscall_id] > 0) {
    // 如果 RF_table 中对应的 syscall_id 可被调用的次数 > 0
    // 且是在退出 syscall 的时候, 那么次数减 1
    if (in_syscall == false)
      RF_table[syscall_id]--;
  } else {
    // RF_table 中 syscall_id 对应的指 < 0, 表示是不限制调用的
  }
  return true;
}

#endif
