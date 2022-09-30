#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#include "core.h"
#include "logger.h"

struct Context {
  /**
   * 代码语言
   */
  JUDGE_CONF::Language language = JUDGE_CONF::Language::CPP;

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

static void print_help_message() {
  printf("catjudge/%s\n", CATJUDGE_VERSION);
  puts("");
  puts("Options:");
  puts("  -d <dir>       Run directory");
  puts("  -l <language>  Code Language");
  puts("  -t <time>      Time limit");
  puts("  -m <memory>    Memory limit");
  puts("  -s <checker>   Checker path");
  exit(JUDGE_CONF::EXIT_HELP);
}

Context* parse_cli_args(int argc, char* argv[]) {
  Context* ctx = new Context();

  char opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "l:t:m:d:s:h")) != -1) {
    switch (opt) {
      case 'h':
        print_help_message();
        break;
      case 'l':
        if (std::isalpha(optarg[0])) {
          if (std::strcmp(optarg, "c")) {
            ctx->language = JUDGE_CONF::Language::C;
          } else if (std::strcmp(optarg, "cpp")) {
            ctx->language = JUDGE_CONF::Language::CPP;
          } else if (std::strcmp(optarg, "java")) {
            ctx->language = JUDGE_CONF::Language::JAVA;
          } else {
            FM_LOG_FATAL("Unknown code language provided: %s", optarg);
            exit(JUDGE_CONF::EXIT_BAD_PARAM);
          }
        } else {
          ctx->language = (JUDGE_CONF::Language) std::atoi(optarg);
          if (ctx->language <= 0 || ctx->language > JUDGE_CONF::Language::JAVA) {
            FM_LOG_FATAL("Unknown code language provided: %s", optarg);
            exit(JUDGE_CONF::EXIT_BAD_PARAM);
          }
        }
        break;
      case 't':
        ctx->time_limit = std::atoi(optarg);
        break;
      case 'm':
        ctx->memory_limit = std::atoi(optarg);
        break;
      case 's':
        ctx->checker = optarg;
        break;
      case 'd':
        ctx->run_dir = optarg;
        break;
      default:
        FM_LOG_WARNING("Unknown option provided: -%c %s", opt, optarg);
        exit(JUDGE_CONF::EXIT_BAD_PARAM);
    }
  }

//  if (has_suffix(PROBLEM::code_path, ".cpp")) {
//      PROBLEM::lang = JUDGE_CONF::LANG_CPP;
//  } else if (has_suffix(PROBLEM::code_path, ".c")) {
//      PROBLEM::lang = JUDGE_CONF::LANG_C;
//  } else if (has_suffix(PROBLEM::code_path, ".java")) {
//      PROBLEM::lang = JUDGE_CONF::LANG_JAVA;
//  } else {
//      FM_LOG_WARNING("It seems that you give me a language which I do not known now: %d", PROBLEM::lang);
//      exit(JUDGE_CONF::EXIT_BAD_PARAM);
//  }

//  PROBLEM::exec_file = PROBLEM::run_dir + "/a.out";
//  PROBLEM::input_file = PROBLEM::run_dir + "/in.in";
//  PROBLEM::output_file = PROBLEM::run_dir + "/out.out";
//  PROBLEM::exec_output = PROBLEM::run_dir + "/out.txt";
//  PROBLEM::result_file = PROBLEM::run_dir + "/result.txt";
//  PROBLEM::stdout_file_compiler = PROBLEM::run_dir + "/stdout_file_compiler.txt";
//  PROBLEM::stderr_file_compiler = PROBLEM::run_dir + "/stderr_file_compiler.txt";
//
//  if (PROBLEM::lang == JUDGE_CONF::LANG_JAVA) {
//    PROBLEM::exec_file = PROBLEM::run_dir + "/Main";
//    PROBLEM::time_limit *= JUDGE_CONF::JAVA_TIME_FACTOR;
//    PROBLEM::memory_limit *= JUDGE_CONF::JAVA_MEM_FACTOR; //Java放宽内存和时间限制
//  }

//  if (PROBLEM::spj) {
//    switch (PROBLEM::spj_lang) {
//      case 1:
//      case 2:
//        PROBLEM::spj_exec_file = PROBLEM::run_dir + "/SpecialJudge";
//        break;
//      case 3:
//        PROBLEM::spj_exec_file = PROBLEM::run_dir + "/SpecialJudge";
//        break;
//      default:
//        FM_LOG_WARNING("OMG, I really do not kwon the special judge problem language.");
//        exit(JUDGE_CONF::EXIT_BAD_PARAM);
//    }
//
//    PROBLEM::spj_output_file = PROBLEM::run_dir + "/spj_output.txt";
//  }

  FM_LOG_DEBUG("Run    dir  : %s", ctx->run_dir);
  FM_LOG_DEBUG("Time   limit: %d", ctx->time_limit);
  FM_LOG_DEBUG("Memory limit: %d", ctx->memory_limit);
  FM_LOG_DEBUG("Output limit: %d", ctx->output_limit);
  FM_LOG_DEBUG("Checker     : %s", ctx->checker);

  return ctx;
}

int main(int argc, char* argv[]) {
  log_open(LOG_PATH);

  Context* ctx = parse_cli_args(argc, argv);

  // 为了构建沙盒，必须要有 root 权限
  if (geteuid() != 0) {
    FM_LOG_FATAL("You must run this program as root.");
    exit(JUDGE_CONF::EXIT_UNPRIVILEGED);
  }

  return 0;
}