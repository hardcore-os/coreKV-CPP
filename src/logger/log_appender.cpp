#include "log_appender.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <iostream>

#include "utils/util.h"
namespace corekv {
namespace {
void GenerateLogFileName(std::string &output) {
  util::GetCurrentTimeString(output);
  output.append(std::to_string(util::GetCurrentPid()));
}
}  // namespace

void EmptyAppender::Append(const char *msg, int32_t len) {
  (void)msg;
  (void)len;
}

void ConsoleAppender::Append(const char *msg, int32_t) {
  if (nullptr != msg) {
    std::cout << msg << std::endl;
  }
}

//文件Appender
void AysncFileAppender::Append(const char *msg, int32_t len) {
  if (!msg || len == 0 || fd_ == -1) {
    return;
  }

  std::lock_guard<std::mutex> lck(mutex_);
  int32_t ret = write(fd_, msg, len);
  cur_file_size += ret;
  if (log_config_.rotate_size > 0 && cur_file_size > log_config_.rotate_size) {
    this->Rotate();
  }
}

AysncFileAppender::AysncFileAppender(const LogConfig &log_config) {
  log_config_ = log_config;
  //构建fd
  int32_t fd = Open();
  if (fd != -1) {
    fd_ = fd;
  }
}
AysncFileAppender::~AysncFileAppender() { Close(); }
void AysncFileAppender::Rotate() {
  if (log_config_.log_type != LogType::FILE) {
    return;
  }
  //滚动生成新的
  std::string output;
  GenerateLogFileName(output);
  static const std::string kDefaultLogPrefix = "corekv.";
  const auto &new_filename =
      log_config_.log_path + kDefaultLogPrefix + output + ".log";

  //新创建的文件，所以重新计算
  cur_file_size = 0;
  int fd = open(active_file_name_.data(), O_CREAT | O_WRONLY | O_APPEND, 0644);
  if (fd == -1) {
    fprintf(stderr, "open log file %s error - %s\n", active_file_name_.c_str(),
            strerror(errno));
    return;
  }
  Close();
  fd_ = fd;

  rename(active_file_name_.c_str(), new_filename.c_str());
}
void AysncFileAppender::Close() {
  if (fd_ != -1) {
    close(fd_);
    fd_ = -1;
  }
}
int32_t AysncFileAppender::Open() {
  if (log_config_.log_type != LogType::FILE) {
    return -1;
  }
  Close();
  if (log_config_.log_path.back() != '/') {
    log_config_.log_path.append(1, '/');
  }
  if (0 != access(log_config_.log_path.c_str(), 0)) {
    // if this folder not exist, create a new one.
    mkdir(log_config_.log_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  }

  active_file_name_ = log_config_.log_path + kLogActiveName;
  int fd = open(active_file_name_.data(), O_CREAT | O_WRONLY | O_APPEND, 0644);
  if (fd == -1) {
    fprintf(stderr, "open log file %s error - %s\n", active_file_name_.c_str(),
            strerror(errno));
    return -1;
  }
  //获取当前文件的大小(可能又重新打开)
  struct stat st;
  int32_t ret = fstat(fd, &st);
  if (ret == -1) {
    fprintf(stderr, "fstat log file %s error!\n", active_file_name_.c_str());
    return -1;
  } else {
    cur_file_size = st.st_size;
  }
  return fd;
}
}  // namespace corekv
