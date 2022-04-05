#include "log_level.h"

#include <algorithm>
namespace corekv {
std::string LogLevelToString(LogLevel log_level) {
  switch (log_level) {
    case LogLevel::TRACE:
      return "[TRACE] ";
    case LogLevel::INFO:
      return "[INFO] ";
    case LogLevel::WARN:
      return "[WARN] ";
    case LogLevel::ERROR:
      return "[ERROR] ";
    case LogLevel::FATAL:
      return "[FATAL] ";
    default:
      return "[TRACE] ";
  }
}
LogLevel LogLevelFromString(const std::string& log_level_str) {
  std::string output;
  std::transform(log_level_str.begin(), log_level_str.end(), output.begin(),
                 ::toupper);

  if (output == "TRACE") {
    return LogLevel::TRACE;
  } else if (output == "INFO") {
    return LogLevel::INFO;
  } else if (output == "WARN") {
    return LogLevel::WARN;
  } else if (output == "ERROR") {
    return LogLevel::ERROR;
  } else if (output == "FATAL") {
    return LogLevel::FATAL;
  }
  return LogLevel::TRACE;
}

}  // namespace corekv
