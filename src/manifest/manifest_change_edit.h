#pragma once
#include <string>
#include <vector>
namespace corekv {
struct Manifest;
enum ManifestChanageOpType { kCreate = 0, kDelete = 1 };
// 主要是用来写磁盘的
struct ManifestChanage {
  uint64_t id;
  uint32_t level;
  // 默认为创建
  ManifestChanageOpType manifest_change_type = ManifestChanageOpType::kCreate;
};

class ManifestChangeEdit {
 public:
  void ParseFromManifest(const Manifest& manifest);

  void EncodeTo(const std::vector<ManifestChanage>& manifest_changes,
                std::string* out);
  void DecodeTo(const std::string& input);
  void ApplyChangeSet(Manifest& manifest);
  const std::vector<ManifestChanage>& GetManifestChanages() {
    return manifest_changes_;
  }

 private:
  std::vector<ManifestChanage> manifest_changes_;
};
}  // namespace corekv
