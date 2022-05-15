#pragma once
#include <stdint.h>

#include <memory>
namespace corekv {
class FilterPolicy;
class Comparator;
}
namespace corekv {

enum class BlockCompressType : uint8_t {
  kNonCompress = 0,
  kSnappyCompression = 1
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
};
}  // namespace corekv