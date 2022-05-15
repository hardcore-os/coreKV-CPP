#include "block_builder.h"

#include "../utils/codec.h"
namespace corekv {
using namespace util;

DataBlockBuilder::DataBlockBuilder(const Options* options) : options_(options) {
  restarts_.emplace_back(0);
}
// 先写完key和value之后，需要将restart_pointer数据保存进行
// restart_pointer我们使用固定长度，方便我们快速恢复出数据
void DataBlockBuilder::AddRestartPointers() {
  if (is_finished_) {
    return;
  }
  for (const auto& restart : restarts_) {
    PutFixed32(&buffer_, restart);
  }
  PutFixed32(&buffer_, restarts_.size());
  is_finished_ = true;
}
void DataBlockBuilder::Finish() { AddRestartPointers(); }
void DataBlockBuilder::Add(const std::string_view& key,
                           const std::string_view& value) {
  if (is_finished_ || key.empty()) {
    return;
  }
  // 这个是写数据部分
  int32_t shared = 0;
  const auto& current_key_size = key.size();
  // 如果小于interval，此时我们需要进行前缀压缩
  if (restart_pointer_counter_ < options_->block_restart_interval) {
    // 当前key和前一个key的公共部分，
    const auto& pre_key_size = pre_key_.size();

    while (shared < pre_key_size && shared < current_key_size &&
           pre_key_[shared] == key[shared]) {
      ++shared;
    }
  } else {
    // restart记录的是二进制数据的偏移量
    restarts_.emplace_back(buffer_.size());
    restart_pointer_counter_ = 0;
  }
  const auto& non_shared_size = current_key_size - shared;
  // <shared><non_shared><value>
  // user_key|ts(内部创建)|
  // value_struct[meta(type[del|add|value_ptr])|ExpiresAt(ttl)|value]
  const auto& value_size = value.size();
  PutVarint32(&buffer_, shared);
  PutVarint32(&buffer_, non_shared_size);
  PutVarint32(&buffer_, value_size);
  // 将当前的key信息序列化到buffer中
  buffer_.append(key.data() + shared, non_shared_size);
  buffer_.append(value.data(), value_size);
  // 更新pre_key，因为下次我们需要使用他
  pre_key_.assign(key.data(), current_key_size);
  ++restart_pointer_counter_;
}

// filter_block_builder
FilterBlockBuilder::FilterBlockBuilder(const Options& options) {
  if (options.filter_policy) {
    policy_filter_ = options.filter_policy.get();
  }
}
void FilterBlockBuilder::Add(const std::string_view& key) {
  if (key.empty() || !Availabe()) {
    return;
  }
  datas_.emplace_back(key);
}
void FilterBlockBuilder::CreateFilter() {
  if (!Availabe() || datas_.empty()) {
    return;
  }
  policy_filter_->CreateFilter(&datas_[0], datas_.size());
}
bool FilterBlockBuilder::MayMatch(const std::string_view& key) {
  if (key.empty() || !Availabe()) {
    return false;
  }
  return policy_filter_->MayMatch(key, 0, 0);
}
bool FilterBlockBuilder::MayMatch(const std::string_view& key,
                                  const std::string_view& bf_datas) {
  if (key.empty() || !Availabe()) {
    return false;
  }
  return policy_filter_->MayMatch(key, bf_datas);
}
const std::string& FilterBlockBuilder::Data() { return buffer_; }
void FilterBlockBuilder::Finish() {
  if (Availabe() && !datas_.empty()) {
    // 先构建布隆过滤器
    CreateFilter();
    // // 序列化hash个数和bf本身数据
    buffer_ = policy_filter_->Data();
    util::PutFixed32(&buffer_, policy_filter_->GetMeta().hash_num);
  }
}
}  // namespace corekv
