#pragma once
#include <functional>

#include "cache_node.h"
namespace corekv {
template <typename KeyType, typename ValueType>
class CachePolicy {
 public:
  virtual ~CachePolicy() = default;
  virtual void Insert(const KeyType& key, ValueType* value,
                      uint32_t ttl = 0) = 0;
  virtual CacheNode<KeyType, ValueType>* Get(const KeyType& key) = 0;
  virtual void Release(CacheNode<KeyType, ValueType>* node) = 0;
  virtual void Prune() = 0;
  virtual void Erase(const KeyType& key) = 0;
  virtual void RegistCleanHandle(
      std::function<void(const KeyType& key, ValueType* value)> destructor) = 0;
};
}  // namespace corekv
