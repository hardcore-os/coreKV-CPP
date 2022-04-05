#include "log.h"

#include <stdarg.h>
#include <stdlib.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "utils/util.h"

#include "log_config.h"
#include "log_level.h"
namespace corekv {
void Log::InitLog(const LogConfig &log_config) {
  if (inited_) {
    return;
  }
  inited_ = true;
  log_config_ = log_config;
  switch (log_config_.log_type) {
    case LogType::CONSOLE:
      log_appender_ = std::make_unique<ConsoleAppender>();
      break;
    case LogType::FILE:
      log_appender_ = std::make_unique<AysncFileAppender>(log_config_);
      break;
    default:
      log_appender_ = std::make_unique<EmptyAppender>();
      break;
  }
}

void Log::LogV(LogLevel log_level, const char *fmt, ...) {
  if (log_level < log_config_.log_level) {
    return;
  }
   
  //先追加个时间
  std::string buffer(1,'\n');
  util::GetCurrentTimeString(buffer);
  buffer.append(LogLevelToString(log_level));
  int32_t len = buffer.size();

  std::string output(log_config_.log_buffer_max_size - len,0);
  va_list ap;
  va_start(ap, fmt);
  int32_t fmt_len = vsnprintf(&output[0],
                              log_config_.log_buffer_max_size - len, fmt, ap);
  va_end(ap);
  buffer+=output;
  log_appender_->Append(&buffer[0], len+fmt_len);
}

}  // namespace corekv
