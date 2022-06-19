#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace corekv {
class options;

// 记录的是manifest中sst元数据信息
struct TableManifest {
  uint32_t level;
  // crc_sum留作扩展
  uint64_t crc_sum;
};
// manifest：主要用于内存中使用
struct Manifest {
  // vector对象组成为下标为level,value为set<table_id>
  std::vector<std::unordered_set<uint64_t>> level_tables_map;
  //某个sst属于哪一层,主要用于加速查询的作用
  std::unordered_map<uint64_t, TableManifest> table_levels_map;
  // 创建操作次数
  int32_t creations = 0;
  // 删除次数
  int32_t deletions = 0;
  void Clear() {
    level_tables_map.clear();
    table_levels_map.clear();
    creations = 0;
    deletions = 0;
  }
};
// 主要是用来读写manifest文件
class ManifestHandler {
 public:
  ManifestHandler(const std::string& db_path);
  bool OpenManifestFile();
  bool ReWrite();
  //
  bool AddChanges(const std::string& input);
  bool AddTableMeta(int32_t level, int64_t sst_id);
  bool RevertToManifest(const std::unordered_set<uint64_t>& delete_sst_ids);
  const Manifest& GetManifest() { return manifest_; }

 private:
  void CreateNewManifestFile();
  bool RecoverFromReWriteManifestFile();
  bool ReplayManifestFile();
 private:
  Manifest manifest_;
  std::string db_path_;
};
}  // namespace corekv
