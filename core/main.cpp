#include <ctime>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#include "conf.h"
#include "logger.h"
#include "context.h"
#include "sandbox.cpp"

static Context *global_context = nullptr;

/*
 * 输出判题结果到结果文件
 */
static void output_result() {
  if (global_context == nullptr) return;
  if (global_context->result == nullptr) return;

  FILE *result_file = fopen(global_context->result_file().c_str(), "w");

  std::string status;
  switch (global_context->result->verdict) {
    case Verdict::CE:
      status = "CompileError";
      break;
    case Verdict::TLE:
      status = "TimeLimitExceeded";
      break;
    case Verdict::MLE:
      status = "MemoryLimitExceeded";
      break;
    case Verdict::OLE:
      status = "OutputLimitExceeded";
      break;
    case Verdict::RE:
      status = "RuntimeError";
      break;
    case Verdict::WA:
      status = "WrongAnswer";
      break;
    case Verdict::AC:
      status = "Accepted";
      break;
    case Verdict::PE:
      status = "PresentationError";
      break;
    default:
      status = "SystemError";
      break;
  }

  fprintf(result_file, "status         %s\n", status.c_str());
  fprintf(result_file, "time           %ld\n", global_context->result->time_user);
  fprintf(result_file, "memory         %ld\n", global_context->result->memory);
  fprintf(result_file, "checker_time   %ld\n", global_context->result->checker_time);
  fprintf(result_file, "checker_memory %ld\n", global_context->result->checker_memory);
  fprintf(result_file, "\n%s", global_context->result->checker_stderr.c_str());
  fprintf(result_file, "\n%s", global_context->result->checker_stdout.c_str());

  FM_LOG_TRACE("Verdict        : %s", status.c_str());
  FM_LOG_TRACE("Time           : %ld ms", global_context->result->time);
  FM_LOG_TRACE("Time User      : %ld ms", global_context->result->time_user);
  FM_LOG_TRACE("Time Sys       : %ld ms", global_context->result->time_sys);
  FM_LOG_TRACE("Memory         : %ld KB", global_context->result->memory);
  FM_LOG_TRACE("Checker Time   : %ld ms", global_context->result->checker_time);
  FM_LOG_TRACE("Checker Memory : %ld KB", global_context->result->checker_memory);
}

static void print_help_message() {
  printf("catjudge/%s\n", CATJUDGE_VERSION);
  puts("");
  puts("Options:");
  puts("  -d <dir>       Run directory");
  puts("  -s <checker>   Checker path");
  puts("  -l <language>  Code language (c / cpp / java)");
  puts("  -t <time>      Time limit (unit: ms)");
  puts("  -m <memory>    Memory limit (unit: KB)");
  exit(CONF::EXIT::HELP);
}

static void print_version_message() {
  printf("catjudge/%s\n", CATJUDGE_VERSION);
//  printf("\n");
//  printf("Checker Root     %s\n", CHECKER_ROOT);
//  printf("Default Checker  %s\n", DEFAULT_CHECKER);
  exit(CONF::EXIT::HELP);
}

Context *parse_cli_args(int argc, char *argv[]) {
  Context *ctx = new Context();

  char opt;
  extern char *optarg;

  while ((opt = getopt(argc, argv, "l:t:m:d:s:hv")) != -1) {
    switch (opt) {
      case 'h':
        print_help_message();
        break;
      case 'v':
        print_version_message();
        break;
      case 'l':
        if (std::isalpha(optarg[0])) {
          if (std::strcmp(optarg, "c") == 0) {
            ctx->language = CONF::Language::C;
          } else if (std::strcmp(optarg, "cpp") == 0) {
            ctx->language = CONF::Language::CPP;
          } else if (std::strcmp(optarg, "java") == 0) {
            ctx->language = CONF::Language::JAVA;
          } else {
            FM_LOG_FATAL("Unknown code language provided: %s", optarg);
            exit(CONF::EXIT::BAD_PARAM);
          }
        } else {
          ctx->language = (CONF::Language) std::atoi(optarg);
          if (ctx->language <= 0 || ctx->language > CONF::LanguageCount) {
            FM_LOG_FATAL("Unknown code language provided: %s", optarg);
            exit(CONF::EXIT::BAD_PARAM);
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
        exit(CONF::EXIT::BAD_PARAM);
    }
  }

  FM_LOG_DEBUG("Run dir     : %s", ctx->run_dir);
  FM_LOG_TRACE("Language    : %s (%d)", CONF::LanguageStr[ctx->language], ctx->language);
  FM_LOG_TRACE("Time limit  : %d ms", ctx->time_limit);
  FM_LOG_TRACE("Memory limit: %d KB", ctx->memory_limit);
  FM_LOG_TRACE("Output limit: %d KB", ctx->output_limit);
  FM_LOG_TRACE("Checker     : %s", ctx->checker);

  return ctx;
}

void start_log() {
  static char path[1024];

  time_t curTime = time(nullptr);
  struct tm *localTime = localtime(&curTime);
  int year = localTime->tm_year + 1900; // Year is # years since 1900
  int month = localTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept
  int day = localTime->tm_mday;

  sprintf(path, "%s/catj-%d-%d-%d.log", LOG_PATH, year, month, day);
  log_open(path);
}

int main(int argc, char *argv[]) {
  start_log();

  // 退出程序时的回调函数，用于输出判题结果
  atexit(output_result);

  Context *ctx = parse_cli_args(argc, argv);
  global_context = ctx;

  // 为了构建沙盒，必须要有 root 权限
  if (geteuid() != 0) {
    FM_LOG_FATAL("You must run this program as root.");
    exit(CONF::EXIT::UNPRIVILEGED);
  }

  FM_LOG_TRACE("Start judging");
  judge(ctx);

  return CONF::EXIT::OK;
}
