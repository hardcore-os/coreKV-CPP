#pragma once
#include "../db/options.h"
#include "../file/file.h"
#include "block_builder.h"
#include "offset_size.h"
namespace corekv {
struct Options;
}  // namespace corekv

namespace corekv {
class TableBuilder final {
 public:
  TableBuilder(const TableBuilder&) = delete;
  TableBuilder& operator=(const TableBuilder&) = delete;
  TableBuilder(const Options& options,FileWriter* file_handler);
  void Add(const std::string_view& key, const std::string_view& value);
  // Finish是指Add最后，有一部分数据还没来得及刷盘
  void Finish();
  bool Success() { return status_ == Status::kSuccess; }
  uint32_t GetFileSize() {
    return block_offset_;
  }
  uint32_t GetEntryNum() {
    return entry_count_;
  }
 private:
  void Flush();
  void WriteDataBlock(DataBlockBuilder& data_block, OffSetSize& offset_size);
  void WriteBytesBlock(const std::string& datas,
                       BlockCompressType block_compress_type,
                       OffSetSize& offset_size);

 private:
  Options options_;
  // index block部分不需要进行差值压缩，因为本身数据就很少
  Options index_options_;
  DataBlockBuilder data_block_builder_;
  DataBlockBuilder index_block_builder_;
  FilterBlockBuilder filter_block_builder_;
  OffsetBuilder index_block_offset_size_builder_;
  FileWriter* file_handler_ = nullptr;
  // 索引的构建，我们只能在下一个block开始时候去构建上一个
  std::string pre_block_last_key_;
  // 前一个block偏移量和大小的话
  OffSetSize pre_block_offset_size_;
  uint32_t block_offset_ = 0;
  uint64_t entry_count_ = 0;
  bool need_create_index_block_ = false;
  DBStatus status_;
};
}  // namespace corekv
