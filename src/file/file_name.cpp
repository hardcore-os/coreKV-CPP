#include "file_name.h"
namespace corekv {
namespace {

std::string MakeFileName(uint64_t number, const char* suffix) {
  char buf[100] = {0};
  std::snprintf(buf, sizeof(buf), "%06llu.%s",
                static_cast<unsigned long long>(number), suffix);
  return buf;
}
}  // namespace
std::string FileName::DescriptorFileName(const std::string& dbname,
                                         const std::string& file_name) {
  if (!dbname.empty()) {
    std::string ch = dbname.back() != '/' ? "/" : "";
    return dbname + ch + file_name;
  }
  // 如果没设置，那就放在当前路径
  return file_name;
}
std::string FileName::FileNameSSTable(const std::string& dbname,
                                      uint64_t sst_id) {
  std::string file_name = MakeFileName(sst_id, "sst");
  if (!dbname.empty()) {
    std::string ch = dbname.back() != '/' ? "/" : "";
    return dbname + ch + file_name;
  }
  return file_name;
}
}  // namespace corekv