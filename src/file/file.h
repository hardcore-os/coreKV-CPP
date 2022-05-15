#pragma once
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

#include "../db/status.h"
namespace corekv {

class FileWriter final {
 public:
  FileWriter(const std::string& file_name);
  ~FileWriter();
  DBStatus Append(const char* data, int32_t len);

  DBStatus FlushBuffer();
  void DeleteFile();
  void Sync();
  void Close();
 private:
  ssize_t Writen(const char* data, int len);

 private:
  static constexpr uint32_t kMaxFileBufferSize = 65536;
  char buffer_[kMaxFileBufferSize];
  int32_t current_pos_ = 0;
  int fd_ = -1;
  std::string file_name_;
};

class FileReader final {
 public:
  ~FileReader();
  FileReader(const std::string& file_name);
  DBStatus Read(uint64_t offset, size_t n, std::string* result) const;

 private:
  int fd_=-1;
};

class FileTool final {
  public:
  static uint64_t GetFileSize(const std::string_view& path);
};
}  // namespace corekv
