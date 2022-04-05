#ifndef LOGGER_LOG_APPENDER_H_
#define LOGGER_LOG_APPENDER_H_
#include <stdint.h>
#include <mutex>
#include "log_config.h"
namespace corekv {
class LogAppender {
 public:
  LogAppender() = default;
  virtual ~LogAppender() = default;
  virtual void Append(const char *msg, int32_t len) = 0;
};
//空的，啥都不输出
class EmptyAppender final: public LogAppender {
 public:
  virtual void Append(const char *msg, int32_t len) override;
};
//控制台输出
class ConsoleAppender final : public LogAppender {
 public:
  virtual void Append(const char *msg, int32_t len) override;
};

//文件Appender
class AysncFileAppender final : public LogAppender {
 public:
  AysncFileAppender(const LogConfig &log_config);
  virtual ~AysncFileAppender();
  virtual void Append(const char *msg, int32_t len) override;
private:
  int32_t Open();
  void Rotate();
  void Close();
 private:
  LogConfig log_config_;
  //当前log file的打开的fd，析构时候记得释放
  int fd_ = -1;
  uint64_t cur_file_size = 0;
  std::mutex mutex_;
  std::string active_file_name_;
};
//后面在支持
// class SocketAppender : public LogAppender {
//  public:
//   virtual void Append(const char *msg, int len) override;
// };
}  // namespace corekv

#endif