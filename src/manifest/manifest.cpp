#include "manifest.h"

#include "../file/file.h"
#include "../file/file_name.h"
#include "../logger/log.h"
#include "manifest_change_edit.h"
#include "manifest_options.h"
namespace corekv {

ManifestHandler::ManifestHandler(const std::string& db_path)
    : db_path_(db_path) {}
// db_path：参数是db路径，该参数应该是db::Open参数设置的
bool ManifestHandler::OpenManifestFile() {
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
      LOG(WARN,"manifest_name=%s", manifest_name.c_str());

    FileWriter file_writer(manifest_name, true);
    file_writer.Append(input.data(), input.size());
    file_writer.Sync();
    file_writer.Close();
  }
  return true;
}

bool ManifestHandler::AddTableMeta(int32_t level, int64_t sst_id) {
  std::vector<ManifestChanage> manifest_changes;
  ManifestChanage manifest_change;
  manifest_change.id = sst_id;
  manifest_change.level = level;
  manifest_changes.emplace_back(manifest_change);
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