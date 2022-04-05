#ifndef LOGGER_LOG_CONFIG_H_
#define LOGGER_LOG_CONFIG_H_
#include <stdint.h>

#include <string>
#include "log_level.h"
namespace corekv {
static constexpr uint32_t kDefaultLogBufferMaxSize = 4096;
static const std::string kLogActiveName = "corekv_active.log";
enum class LogType : uint8_t { EMPTY = 0, CONSOLE = 1, FILE = 2 };

struct LogConfig {
  // log_path默认
  std::string log_path = ".";
  LogType log_type = LogType::EMPTY;
  uint32_t log_buffer_max_size = kDefaultLogBufferMaxSize;
  LogLevel log_level = LogLevel::DEBUG;
  int32_t rotate_size = 100*1024*1024;//默认100M 
};

static std::string LogTypeToString(LogType log_level);
static LogType LogTypeFromString(const std::string& log_type_str); 
}  // namespace corekv

#endif