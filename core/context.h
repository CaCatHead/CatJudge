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
   * 内存使用量, 单位: KB
   */
  long int memory = 0;

  long int checker_memory = 0;

  /**
   * 时间使用量, 单位: ms
   */
  long int time = 0;

  long int checker_time = 0;

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
  int memory_limit = 262144;

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

  std::string checker_name() const {
    int len = strlen(this->checker);
    int st = 0;
    for (int i = 0; i < len; i++) {
      if (this->checker[i] == '/') {
        st = i + 1;
      }
    }
    return std::string(this->checker).substr(st);
  }

  std::string input_file() const {
    return std::string(this->run_dir) + "/in.txt";
  }

  std::string result_file() const {
    return std::string(this->run_dir) + "/result.txt";
  }

  std::string output_file() const {
    return std::string(this->run_dir) + "/sub.out";
  }

  std::string answer_file() const {
    return std::string(this->run_dir) + "/ans.txt";
  }

  std::string error_file() const {
    return std::string(this->run_dir) + "/sub.err";
  }

  std::string checker_output_file() const {
    return std::string(this->run_dir) + "/chk.out";
  }

  std::string checker_error_file() const {
    return std::string(this->run_dir) + "/chk.err";
  }
};

#endif //CATJUDGE_CONTEXT_H
