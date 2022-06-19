#pragma once
#include <stdint.h>
#include <string>
#include <string_view>
#include <vector>

#include "../db/status.h"
namespace corekv {

class FileWriter final {
 public:
  FileWriter(const std::string& file_name, bool append = false);
  ~FileWriter();
  DBStatus Append(const char* data, int32_t len);

  DBStatus FlushBuffer();
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
  static bool Exist(std::string_view path );
  static bool Rename(std::string_view from, std::string_view to);
  static bool RemoveFile(const std::string& file_name);
  static bool RemoveDir(const std::string& dirname);
  static bool CreateDir(const std::string& dirname);
};
}  // namespace corekv
