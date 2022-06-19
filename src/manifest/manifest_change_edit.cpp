#include "manifest_change_edit.h"

#include <string_view>

#include "../utils/codec.h"
#include "manifest.h"
#include "../logger/log.h"
namespace corekv {
using namespace corekv::util;
void ManifestChangeEdit::ParseFromManifest(const Manifest& manifest) {
  const auto table_size = manifest.table_levels_map.size();
  if (table_size > 0) {
    manifest_changes_.resize(table_size);
    for (const auto& item : manifest.table_levels_map) {
      ManifestChanage manifest_change;
      manifest_change.id = item.first;
      manifest_change.level = item.second.level;
      // 就地构造，比std::move性能会更高
      manifest_changes_.emplace_back(manifest_change);
    }
  }
}

void ManifestChangeEdit::EncodeTo(
    const std::vector<ManifestChanage>& manifest_changes, std::string* out) {
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

void ManifestChangeEdit::DecodeTo(const std::string& input) {
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
      ManifestChanage manifest_change;
      manifest_change.id = id;
      manifest_change.level = level;
      manifest_change.manifest_change_type = (ManifestChanageOpType)change_op_type;
      manifest_changes_.emplace_back(manifest_change);
    }
  }
}
void ManifestChangeEdit::ApplyChangeSet(Manifest& manifest) {
  for (const auto& item : manifest_changes_) {
    switch (item.manifest_change_type) {
      case ManifestChanageOpType::kCreate: {
        manifest.table_levels_map[item.id].level = item.level;
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
}  // namespace corekv