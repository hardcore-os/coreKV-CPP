#include "manifest.h"

#include <string_view>

#include "../file/file.h"
#include "../file/file_name.h"
#include "../logger/log.h"
#include "../utils/codec.h"
#include "options.h"
namespace corekv {
using namespace corekv::util;
enum ManifestChanageOpType { kCreate = 0, kDelete = 1 };
// 主要是用来写磁盘的
struct ManifestChanage {
  uint64_t id;
  uint32_t level;
  // 默认为创建
  ManifestChanageOpType manifest_change_type = ManifestChanageOpType::kCreate;
  ManifestChanage(uint64_t id, uint32_t level,
                  ManifestChanageOpType manifest_change_type =
                      ManifestChanageOpType::kCreate) {
    this->id = id;
    this->level = level;
    this->manifest_change_type = manifest_change_type;
  }
};

class ManifestChangeEdit {
 public:
  void ParseFromManifest(const Manifest& manifest) {
    const auto table_size = manifest.table_levels_map.size();
    if (table_size > 0) {
      manifest_changes_.resize(table_size);
      for (const auto& item : manifest.table_levels_map) {
        // 就地构造，比std::move性能会更高
        manifest_changes_.emplace_back(item.first, item.second);
      }
    }
  }

  void EncodeTo(const std::vector<ManifestChanage>& manifest_changes,
                std::string* out) {
    if (!manifest_changes.empty()) {
      // 先保存有多少条记录
      PutVarint32(out, manifest_changes.size());
      for (const auto& item : manifest_changes) {
        PutVarint64(out, item.id);
        PutVarint32(out, item.level);
        // 这里是变长32位，空间压缩比很高
        PutVarint32(out, item.manifest_change_type);
      }
    }
  }
  void DecodeTo(const std::string& input) {
    if (input.empty()) {
      return;
    }
    std::string_view st = input;
    uint32_t record_size = 0;
    GetVarint32(&st, &record_size);
    // 这里需要思考一下，如果写到一半，没写完怎么处理呢？
    for (uint32_t index = 0; index < record_size; ++index) {
      uint64_t id = 0;
      uint32_t level = 0;
      uint32_t change_op_type = 0;
      if (GetVarint64(&st, &id) && GetVarint32(&st, &level) &&
          GetVarint32(&st, &change_op_type)) {
        manifest_changes_.emplace_back(id, level, change_op_type);
      }
    }
  }
  void ApplyChangeSet(Manifest& manifest) {
    for (const auto& item : manifest_changes_) {
      switch (item.manifest_change_type) {
        case ManifestChanageOpType::kCreate: {
          manifest.table_levels_map[item.id] = TableManifest(item.level);
          // item.level中的level下标可能是0也有可能是1，这里+1保证安全
          if (manifest.level_tables_map.size() < item.level) {
            manifest.level_tables_map.resize(item.level + 1);
          }
          manifest.level_tables_map[item.level].insert(item.id);
          ++manifest.creations;
          break;
        }
        case ManifestChanageOpType::kDelete: {
          // 先定位到他是在哪一层
          const auto& iter = manifest.table_levels_map.find(item.id);
          if (iter == manifest.table_levels_map.cend()) {
            LOG(WARN, "don't find id[%ld] in manifest.table_levels_map!",
                item.id);
            return;
          }
          if (manifest.level_tables_map.size() < iter->second.level) {
            manifest.level_tables_map[iter->second.level].erase(item.id);
            manifest.table_levels_map.erase(item.id);
          }
          break;
        }
        default:
          LOG(ERROR, "undefine change op type[%d]!", item.manifest_change_type);
          break;
      }
    }
  }
  const std::vector<ManifestChanage>& GetManifestChanages() {
    return manifest_changes_;
  }

 private:
  std::vector<ManifestChanage> manifest_changes_;
};
ManifestHandler::ManifestHandler(const std::string& db_path)
    : db_path_(db_path) {}
// db_path：参数是db路径，该参数应该是db::Open参数设置的
bool ManifestHandler::OpenManifestFile(const options& options) {
  const auto& manifest_name =
      FileName::DescriptorFileName(db_path_, ManifestOptions::kManifestName);
  if (!FileTool::Exist(manifest_name)) {
    CreateNewManifestFile();
    const auto& res = RecoverFromReWriteManifestFile();
    if (!res) {
      return res;
    }
  }
  return ReplayManifestFile();
}
// 重放manifest文件
bool ManifestHandler::ReplayManifestFile() {
  const auto& manifest_name =
      FileName::DescriptorFileName(db_path_, ManifestOptions::kManifestName);

  CreateNewManifestFile();
  const auto& size = FileTool::GetFileSize(manifest_name);
  if (size == 0) {
    return false;
  }
  FileReader file_reader(manifest_name);
  std::string content;
  file_reader.Read(0, size, &content);
  ManifestChangeEdit manifest_change_edit;
  manifest_change_edit.DecodeTo(content);
  manifest_change_edit.ApplyChangeSet(manifest_);
  return true;
}
bool ManifestHandler::RecoverFromReWriteManifestFile() {
  // 如果manifest文件不存在，那么我们从rewrite_manifest中恢复出来
  const auto& manifest_rewrite_name = FileName::DescriptorFileName(
      db_path_, ManifestOptions::kManifestRewriteFilename);
  // 将当前的manifest序列化之后追加到rewrite中
  ManifestChangeEdit manifest_change_edit;
  manifest_change_edit.ParseFromManifest(manifest_);
  std::string current_manifest_str;
  manifest_change_edit.EncodeTo(manifest_change_edit.GetManifestChanages(),
                                &current_manifest_str);
  if (!current_manifest_str.empty()) {
    FileWriter file_writer(manifest_rewrite_name, true);
    file_writer.Append(current_manifest_str.data(),
                       current_manifest_str.size());
    file_writer.Sync();
    file_writer.Close();
  }
  const auto& manifest_name =
      FileName::DescriptorFileName(db_path_, ManifestOptions::kManifestName);

  return FileTool::Rename(manifest_rewrite_name, manifest_name);
}
void ManifestHandler::CreateNewManifestFile() {
  // 创建一个新的对象
  manifest_.Clear();
}

bool ManifestHandler::ReWrite() {
  const auto code = RecoverFromReWriteManifestFile();
  if (!code) {
    return code;
  }
  manifest_.creations = manifest_.table_levels_map.size();
  manifest_.deletions = 0;
  return true;
}
// 该函数是否需要加锁，在后续逻辑会进行补充
bool ManifestHandler::AddChanges(const std::string& input) {
  if (input.empty()) {
    return false;
  }
  ManifestChangeEdit manifest_change_edit;
  manifest_change_edit.DecodeTo(input);
  manifest_change_edit.ApplyChangeSet(manifest_);
  if (manifest_.deletions >
          ManifestOptions::kManifestDeletionsRewriteThreshold &&
      manifest_.deletions > ManifestOptions::kManifestDeletionsRatio *
                                (manifest_.creations - manifest_.deletions)) {
    if (!ReWrite()) {
      return false;
    }
  } else {
    // 那么说明当前的manifest文件并不是很大，直接追加到当前的manifest文件中
    const auto& manifest_name =
        FileName::DescriptorFileName(db_path_, ManifestOptions::kManifestName);

    FileWriter file_writer(manifest_name, true);
    file_writer.Append(input.data(), input.size());
    file_writer.Sync();
    file_writer.Close();
    return true;
  }
}

bool ManifestHandler::AddTableMeta(int32_t level, int64_t sst_id) {
  std::vector<ManifestChanage> manifest_changes;
  manifest_changes.emplace_back(sst_id, level);
  ManifestChangeEdit manifest_change_edit;
  std::string current_manifest_str;
  manifest_change_edit.EncodeTo(manifest_changes, &current_manifest_str);
  return AddChanges(current_manifest_str);
}

bool ManifestHandler::RevertToManifest(
    const std::unordered_set<uint64_t>& delete_sst_ids) {
  if (delete_sst_ids.empty()) {
    return true;
  }
  // 1. Check all files in manifest exist.
  for (const auto& item : manifest_.table_levels_map) {
    if (delete_sst_ids.count(item.first) == 0) {
      LOG(WARN, "file does not exist for table %d", item.first);
      return false;
    }
  }
  // 2. Delete files that shouldn't exist.
  for (const auto& sst_id : delete_sst_ids) {
    // 表示没有引用了，可以删除
    if (manifest_.table_levels_map.count(sst_id) == 0) {
      const auto& sst_filename = FileName::FileNameSSTable(db_path_, sst_id);
      if (!FileTool::RemoveFile(sst_filename)) {
        return false;
      }
    }
  }
  return true;
}
}  // namespace corekv