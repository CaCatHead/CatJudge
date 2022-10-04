#ifndef CATJUDGE_CONF_H
#define CATJUDGE_CONF_H

namespace CONF {
  /**
   * Judge 本身的时间限制, 单位: ms
   */
  const int JUDGE_TIME_LIMIT = 15000;

  /**
   * 程序运行的栈空间大小
   */
  const int STACK_SIZE_LIMIT = 8192;

  const int KILO = 1024;
  const int MEGA = KILO * KILO;
  const int GIGA = KILO * MEGA;

  enum Verdict {
    PROCEED = 0,  // 已经处理并且正确运行退出
    CE = 1,   // 编译错误
    TLE = 2,  // 超时
    MLE = 3,  // 超内存限制
    OLE = 4,  // 输出文件超过大小限制
    RE = 5,   // 运行时错误，包括数组越界、非法调用等
    WA = 6,   // 答案错误
    AC = 7,   // 答案正确
    PE = 8,   // 输出格式错误
    SE = 9,   // System Error，判题过程发生故障
  };

  enum EXIT {
    OK = 0,  // 正常退出
    HELP = 0,   // 输出帮助信息
    UNPRIVILEGED = 1,   // 缺乏权限退出
    BAD_PARAM = 3,   // 参数错误退出
    VERY_FIRST = 4,   // judge程序运行超时退出
    COMPILE = 6,   // 编译错误退出
    PRE_JUDGE = 9,   // 预处理错误退出，如文件无法打开，无法fork进程等
    PRE_JUDGE_PTRACE = 10,  // ptrace运行错误退出
    PRE_JUDGE_EXECLP = 11,  // execlp运行错误退出
    SET_LIMIT = 15,  // 设置时间限制错误退出
    SET_SECURITY = 17,  // 设置安全限制错误退出
    JUDGE = 21,  // 程序调用错误退出
    COMPARE = 27,
    COMPARE_SPJ = 30,
    COMPARE_SPJ_FORK = 31,
    TIMEOUT = 36,  // 超时退出
    UNKNOWN = 127, // 不详
  };

  const int LanguageCount = 5;
  const char LanguageStr[LanguageCount + 1][8] = {
      "unknown", "c", "cpp", "java", "python2", "python3"
  };

  enum Language {
    C = 1,
    CPP = 2,
    JAVA = 3,
    PYTHON2 = 4,
    PYTHON3 = 5,
  };
}

#endif //CATJUDGE_CONF_H
