#include "data_block.h"

#include "../db/comparator.h"
#include "../utils/codec.h"
namespace corekv {
using namespace util;
// 反解析出来的实际restarts offset个数
uint32_t DataBlock::NumRestarts() const {
  return util::DecodeFixed32(data_ + size_ - sizeof(uint32_t));
}
DataBlock::~DataBlock() {}
DataBlock::DataBlock(const std::string_view& contents)
    : data_(contents.data()), size_(contents.size()), owned_(false) {
  if (size_ < sizeof(uint32_t)) {
    size_ = 0;  // Error marker
  } else {
    // 最后一个保存的是restart总个数，因此最多保留的restart个数(剩余所有的都是restarts
    // offset)
    size_t max_restarts_allowed = (size_ - sizeof(uint32_t)) / sizeof(uint32_t);
    // restart个数
    uint32_t num_restart_size = NumRestarts();
    if (num_restart_size > max_restarts_allowed) {
      size_ = 0;
    } else {
      // 重启点开始的位置，也是数据部分的总长度
      restart_offset_ = size_ - (1 + num_restart_size) * sizeof(uint32_t);
    }
  }
}

static inline const char* DecodeEntry(const char* p, const char* limit,
                                      uint32_t* shared, uint32_t* non_shared,
                                      uint32_t* value_length) {
  if (limit - p < 3) return nullptr;
  *shared = reinterpret_cast<const uint8_t*>(p)[0];
  *non_shared = reinterpret_cast<const uint8_t*>(p)[1];
  *value_length = reinterpret_cast<const uint8_t*>(p)[2];
  if ((*shared | *non_shared | *value_length) < 128) {
    // Fast path: all three values are encoded in one byte each
    p += 3;
  } else {
    if ((p = GetVarint32Ptr(p, limit, shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, non_shared)) == nullptr) return nullptr;
    if ((p = GetVarint32Ptr(p, limit, value_length)) == nullptr) return nullptr;
  }

  if (static_cast<uint32_t>(limit - p) < (*non_shared + *value_length)) {
    return nullptr;
  }
  return p;
}

class DataBlock::Iter : public Iterator {
 private:
  std::shared_ptr<Comparator> comparator_;
  // data是整个数据开始计算的
  const char* const data_;  // underlying block contents
  // 重启点开始的位置
  uint32_t const restarts_;  // Offset of restart array (list of fixed32)

  // 重启点的个数
  uint32_t const num_restarts_;  // Number of uint32_t entries in restart array
  uint32_t current_;
  uint32_t restart_index_;  // Index of restart block in which current_ falls
  std::string key_;
  std::string value_;
  uint32_t offset_ = 0;
  DBStatus status_;

  inline int Compare(const std::string_view& a, const std::string_view& b) {
    return comparator_->Compare(a.data(), b.data());
  }

  // Return the offset in data_ just past the end of the current entry.
  inline uint32_t NextEntryOffset() const {
    return offset_;
    // return (value_.data() + value_.size()) - data_;
  }

  // data+restarts_表示整体restart的起点，根据偏移量来计算出所对应的restart
  // offset 反解析出对应数据的位置
  uint32_t GetRestartPoint(uint32_t index) {
    assert(index < num_restarts_);
    return DecodeFixed32(data_ + restarts_ + index * sizeof(uint32_t));
  }

  void SeekToRestartPoint(uint32_t index) {
    key_.clear();
    // 起始点开始位置
    restart_index_ = index;
    // current_ will be fixed by ParseNextKey();

    // 重启点的位置
    offset_ = GetRestartPoint(index);
    value_ = std::string(data_ + offset_, 0);
  }

 public:
  Iter(std::shared_ptr<Comparator> comparator, const char* data, uint32_t restarts,
       uint32_t num_restarts)
      : comparator_(comparator),
        data_(data),
        restarts_(restarts),
        num_restarts_(num_restarts),
        current_(restarts_),
        restart_index_(num_restarts_) {
    assert(num_restarts_ > 0);
  }
  ~Iter() {
  }
  bool Valid() const override { return current_ < restarts_; }
  DBStatus status() const override { return status_; }
  std::string_view key() const override {
    assert(Valid());
    return key_;
  }
  std::string value() override {
    assert(Valid());
    return value_;
  }

  void Next() override {
    assert(Valid());
    ParseNextKey();
  }

  void Prev() override {
    assert(Valid());

    // Scan backwards to a restart point before current_
    const uint32_t original = current_;
    while (GetRestartPoint(restart_index_) >= original) {
      if (restart_index_ == 0) {
        // No more entries
        current_ = restarts_;
        restart_index_ = num_restarts_;
        return;
      }
      restart_index_--;
    }

    SeekToRestartPoint(restart_index_);
    do {
      // Loop until end of current entry hits the start of original entry
    } while (ParseNextKey() && NextEntryOffset() < original);
  }
  // 根据key二分查找即可
  void Seek(const std::string_view& target) override {
    // Binary search in restart array to find the last restart point
    // with a key < target
    uint32_t left = 0;
    // num_restarts_：表示当前总个数
    uint32_t right = num_restarts_ - 1;

    while (left < right) {
      uint32_t mid = (left + right + 1) / 2;
      // 获取对应的重启点的位置
      uint32_t region_offset = GetRestartPoint(mid);
      uint32_t shared, non_shared, value_length;
      // 反解析出完整的key
      const char* key_ptr =
          DecodeEntry(data_ + region_offset, data_ + restarts_, &shared,
                      &non_shared, &value_length);
      if (key_ptr == nullptr || (shared != 0)) {
        CorruptionError();
        return;
      }
      std::string_view mid_key(key_ptr, non_shared);
      // 比较两个key的大小
      if (Compare(mid_key, target) < 0) {
        // Key at "mid" is smaller than "target".  Therefore all
        // blocks before "mid" are uninteresting.
        left = mid;
      } else {
        // Key at "mid" is >= "target".  Therefore all blocks at or
        // after "mid" are uninteresting.
        right = mid - 1;
      }
    }
    // 定位到当前key所在的重启点的位置，当然有可能不存在，
    // 需要找到第一个key大于等于target的数据
    // Linear search (within restart block) for first key >= target
    SeekToRestartPoint(left);

    while (true) {
      if (!ParseNextKey()) {
        return;
      }

      //找到当前第一个大于等于目标key的实体
      if (Compare(key_, target) >= 0) {
        return;
      }
    }
  }

  void SeekToFirst() override {
    SeekToRestartPoint(0);
    ParseNextKey();
  }

  void SeekToLast() override {
    SeekToRestartPoint(num_restarts_ - 1);
    while (ParseNextKey() && NextEntryOffset() < restarts_) {
      // Keep skipping
    }
  }

 private:
  void CorruptionError() {
    current_ = restarts_;
    restart_index_ = num_restarts_;
    status_ = Status::kInterupt;
    key_.clear();
    value_.clear();
  }

  bool ParseNextKey() {
    // 获取当前重启点数据距离数据起始位置的距离
    current_ = NextEntryOffset();
    const char* p = data_ + current_;
    // 获取整个数据部分
    const char* limit = data_ + restarts_;  // Restarts come right after data
    // 如果超出了限制，那说明前面的二分查找没找到
    if (p >= limit) {
      // No more entries to return.  Mark as invalid.
      current_ = restarts_;
      restart_index_ = num_restarts_;
      return false;
    }
    // 反解析当前key所在的数据索引处
    // Decode next entry
    uint32_t shared, non_shared, value_length;
    p = DecodeEntry(p, limit, &shared, &non_shared, &value_length);
    if (p == nullptr || key_.size() < shared) {
      CorruptionError();
      return false;
    } else {
      key_.resize(shared);
      key_.append(p, non_shared);
      value_ = std::string(p + non_shared, value_length);
      // 更新restart_index_指针，到当前value所在的重启点数据的前一个
      while (restart_index_ + 1 < num_restarts_ &&
             GetRestartPoint(restart_index_ + 1) < current_) {
        ++restart_index_;
      }
      return true;
    }
  }
};

Iterator* DataBlock::NewIterator(std::shared_ptr<Comparator> comparator) {
  if (size_ < sizeof(uint32_t)) {
    return NewErrorIterator(Status::kInterupt);
  }
  const auto num_restarts = NumRestarts();
  if (num_restarts == 0) {
    return NewEmptyIterator();
  } else {
    // restart_offset_：重启点开始的位置，也是数据部分的总长度
    return new Iter(comparator, data_, restart_offset_, num_restarts);
  }
}
}  // namespace corekv
