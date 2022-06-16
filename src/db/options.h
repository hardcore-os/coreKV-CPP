#pragma once
#include <stdint.h>

#include <memory>
#include <string>

#include "../cache/cache.h"
#include "table/data_block.h"
namespace corekv {
class FilterPolicy;
class Comparator;
}
namespace corekv {
  
enum  BlockCompressType  {
  kNonCompress = 0x0,
  kSnappyCompression = 0x1
};

struct Options {
  // 单个block的大小
  uint32_t block_size = 4 * 1024;
  // 16个entry来构建一个restart
  uint32_t block_restart_interval = 16;
  // 最多的层数，默认是7
  uint32_t max_level_num = 7;
  // kv分离的阈值(默认1024k)
  uint32_t max_key_value_split_threshold = 1024;
  // 默认不会进行压缩
  BlockCompressType block_compress_type = BlockCompressType::kNonCompress;

  std::shared_ptr<FilterPolicy> filter_policy = nullptr;

  std::shared_ptr<Comparator> comparator = nullptr;
  Cache<uint64_t, DataBlock>* block_cache = nullptr;
};
struct ReadOptions {

};

// manifest相关的文件
struct ManifestOptions {
  // manifest后缀名
  static const std::string kManifestName = "MANIFEST";
  static const std::string kManifestRewriteFilename = "REWRITEMANIFEST";
  static constexpr uint32_t kManifestDeletionsRewriteThreshold = 10000;
  static constexpr uint32_t kManifestDeletionsRatio = 10;
};
}  // namespace corekv