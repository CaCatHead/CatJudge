#ifndef CATJUDGE_CONTEXT_H
#define CATJUDGE_CONTEXT_H

#include "conf.h"

struct Context {
  /**
   * 代码语言
   */
  CONF::Language language = CONF::Language::CPP;

  /**
   * 时间限制, 单位: ms
   */
  int time_limit = 1000;

  /**
   * 内存限制, 单位: KB
   */
  int memory_limit = 65535;

  /**
   * 输出文件的大小限制, 单位: KB
   */
  int output_limit = 1024000;

  /**
   * Checker
   */
  const char* checker;

  /**
   * 沙盒路径，所有运行过程所在的文件夹， 包括：
   * <ol>
   * <li>测试用例输入文件：in.in</li>
   * <li>测试用例答案文件：out.out</li>
   * <ol/>
   */
  const char* run_dir;
};

#endif //CATJUDGE_CONTEXT_H
