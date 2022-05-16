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
  // 默认不会进行压缩
  BlockCompressType block_compress_type = BlockCompressType::kNonCompress;

  std::shared_ptr<FilterPolicy> filter_policy = nullptr;

  std::shared_ptr<Comparator> comparator = nullptr;
  Cache<uint64_t, DataBlock>* block_cache = nullptr;
};
struct ReadOptions {

};
}  // namespace corekv