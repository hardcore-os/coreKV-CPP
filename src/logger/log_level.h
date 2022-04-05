#ifndef LOGGER_LOG_LEVEL_H_
#define LOGGER_LOG_LEVEL_H_
#include <stdint.h>

#include <string>

#include "../utils/string_util.h"
namespace corekv {
enum LogLevel {
  TRACE = 0,
  DEBUG = 1,
  INFO = 2,
  WARN = 3,
  ERROR = 4,
  FATAL = 5
};
std::string LogLevelToString(LogLevel log_level);
LogLevel LogLevelFromString(const std::string& log_level_str);

}  // namespace corekv

#endif