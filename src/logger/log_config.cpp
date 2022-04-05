#include "log_config.h"

#include <algorithm>

namespace corekv {

std::string LogTypeToString(LogType log_level) {
  switch (log_level) {
    case LogType::FILE:
      return "[FILE]";
    case LogType::CONSOLE:
      return "[CONSOLE]";
    case LogType::EMPTY:
    default:
      return "[EMPTY]";
  }
}
LogType LogTypeFromString(const std::string& log_type_str) {
  std::string output;
  std::transform(log_type_str.begin(), log_type_str.end(), output.begin(),
                 ::toupper);

  if (output == "CONSOLE") {
    return LogType::CONSOLE;
  } else if (output == "FILE") {
    return LogType::FILE;
  }
  return LogType::EMPTY;
}
}  // namespace corekv
