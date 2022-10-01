#ifndef CATJUDGE_CONTEXT_H
#define CATJUDGE_CONTEXT_H

#include <string>

#include "conf.h"

struct Result {
  /**
   * 结果代号
   */
  CONF::Verdict verdict = CONF::Verdict::SE;

  /**
   * 内存使用量, 单位: ?
   */
  int memory = 0;

  /**
   * 时间使用量, 单位: ?
   */
  int time = 0;

  /**
   * 额外信息
   */
  std::string extra_message;

  /**
   * 最终结果
   */
  std::string status;
};

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
  const char *checker = DEFAULT_CHECKER;

  /**
   * 沙盒路径，所有运行过程所在的文件夹， 包括：
   * <ol>
   * <li>测试用例输入文件：in.in</li>
   * <li>测试用例答案文件：out.out</li>
   * <ol/>
   */
  const char *run_dir;

  /**
   * 运行结果
   */
  Result *result = nullptr;

  std::string input_file() const {
    return std::string(this->run_dir) + "/in.in";
  }

  std::string result_file() const {
    return std::string(this->run_dir) + "/result.txt";
  }

  std::string output_file() const {
    return std::string(this->run_dir) + "/out.txt";
  }

  std::string answer_file() const {
    return std::string(this->run_dir) + "/out.out";
  }

  std::string error_file() const {
    return std::string(this->run_dir) + "/err.txt";
  }
};

#endif //CATJUDGE_CONTEXT_H
