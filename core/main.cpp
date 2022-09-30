#include <cctype>
#include <cstdio>
#include <cstring>
#include <cstdlib>

#include <unistd.h>

#include "conf.h"
#include "logger.h"
#include "context.h"
#include "sandbox.cpp"

static void print_help_message() {
  printf("catjudge/%s\n", CATJUDGE_VERSION);
  puts("");
  puts("Options:");
  puts("  -d <dir>       Run directory");
  puts("  -l <language>  Code Language");
  puts("  -t <time>      Time limit");
  puts("  -m <memory>    Memory limit");
  puts("  -s <checker>   Checker path");
  exit(CONF::EXIT::HELP);
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
            ctx->language = CONF::Language::C;
          } else if (std::strcmp(optarg, "cpp")) {
            ctx->language = CONF::Language::CPP;
          } else if (std::strcmp(optarg, "java")) {
            ctx->language = CONF::Language::JAVA;
          } else {
            FM_LOG_FATAL("Unknown code language provided: %s", optarg);
            exit(CONF::EXIT::BAD_PARAM);
          }
        } else {
          ctx->language = (CONF::Language) std::atoi(optarg);
          if (ctx->language <= 0 || ctx->language > CONF::Language::JAVA) {
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
    exit(CONF::EXIT::UNPRIVILEGED);
  }

  judge(ctx);

  return CONF::EXIT::OK;
}