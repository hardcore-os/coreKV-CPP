#pragma once
#include "offset_size.h"
namespace corekv {
// 可以理解为元数据
class Footer final {
 public:
  void EncodeTo(std::string* dst);
  DBStatus DecodeFrom(std::string_view* input);
  void SetFilterBlockMetaData(const OffSetSize& filter_block) {
    filter_block_ = filter_block;
  }
  void SetIndexBlockMetaData(const OffSetSize& index_block) {
    index_block_ = index_block;
  }
  const OffSetSize& GetFilterBlockMetaData() const { return filter_block_; }
  const OffSetSize& GetIndexBlockMetaData() const { return index_block_; }

  std::string DebugString();

 private:
  // filter block部分在整个data block中的偏移量和大小
  OffSetSize filter_block_;
  // index block部分在整个data block中的偏移量和大小
  OffSetSize index_block_;
  OffsetBuilder offset_builder_;
};
}  // namespace corekv
