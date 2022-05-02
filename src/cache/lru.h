#pragma once
#include <functional>
#include <list>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <iostream>

#include "../utils/hash_util.h"
#include "../utils/mutex.h"
#include "../utils/util.h"
#include "cache_node.h"
#include "cache_policy.h"
namespace corekv {
template <typename KeyType, typename ValueType, typename LockType = NullLock>
class LruCachePolicy final : public CachePolicy<KeyType, ValueType> {
  using ListIter = typename std::list<CacheNode<KeyType, ValueType>*>::iterator;

 public:
  LruCachePolicy(uint32_t capacity) : capacity_(capacity) {}
  // 其实走到这里，说明整个程序都结束了
  ~LruCachePolicy() {
    for (auto it = nodes_.begin(); it != nodes_.end(); it++) {
      // Unref(it);
    }
  }
  void Insert(const KeyType& key, ValueType* value, uint32_t ttl = 0) {
    LockType lock_type;
    ScopedLockImpl<LockType> lock_guard(lock_type);
    CacheNode<KeyType, ValueType>* new_node =
        new CacheNode<KeyType, ValueType>();
    new_node->hash = std::hash<KeyType>{}(key);
    new_node->key = key;
    new_node->value = value;
    new_node->in_cache = true;
    new_node->refs = 1;
    new_node->ttl = ttl;
    if (ttl > 0) {
      new_node->last_access_time = util::GetCurrentTime();
    }
    typename std::unordered_map<
        KeyType,
        typename std::list<CacheNode<KeyType, ValueType>*>::iterator>::iterator
        iter = index_.find(key);
    //首先判断是否有相同的key在缓存中
    if (iter == index_.end()) {
      //淘汰最后一个，然后将其加到第一个位置
      if (nodes_.size() == capacity_) {
        CacheNode<KeyType, ValueType>* node = nodes_.back();
        index_.erase(node->key);
        nodes_.pop_back();
        FinishErase(node);
      }
      nodes_.push_front(new_node);
      index_[key] = nodes_.begin();
    } else {
      //说明已经存在有新的值
      //更新节点的值，并将其加到第一个位置
      FinishErase(*(iter->second));
      nodes_.splice(nodes_.begin(), nodes_, index_[key]);
      index_[key] = nodes_.begin();
    }
  }
  CacheNode<KeyType, ValueType>* Get(const KeyType& key) {
    LockType lock_type;
    ScopedLockImpl<LockType> lock_guard(lock_type);
    typename std::unordered_map<
        KeyType,
        typename std::list<CacheNode<KeyType, ValueType>*>::iterator>::iterator
        iter = index_.find(key);
    if (iter == index_.end()) {
      return nullptr;
    }
    CacheNode<KeyType, ValueType>* node = *(iter->second);
    nodes_.erase(iter->second);
    //需要移动到头部
    nodes_.push_front(node);
    index_[node->key] = nodes_.begin();
    Ref(node);
    return node;
  }
  // 默认的析构函数
  void RegistCleanHandle(
      std::function<void(const KeyType& key, ValueType* value)> destructor) {
    destructor_ = destructor;
  }
  void Release(CacheNode<KeyType, ValueType>* node) {
    LockType lock_type;
    ScopedLockImpl<LockType> lock_guard(lock_type);
    Unref(node);
  }
  // 定期的来进行回收
  void Prune() {
    LockType lock_type;
    ScopedLockImpl<LockType> lock_guard(lock_type);
    for (auto it = wait_erase_.begin(); it != wait_erase_.end(); ++it) {
      Unref((it->second));
    }
  }
  // 删除某个key对应的节点
  void Erase(const KeyType& key) {
    LockType lock_type;
    ScopedLockImpl<LockType> lock_guard(lock_type);
    typename std::unordered_map<
        KeyType,
        typename std::list<CacheNode<KeyType, ValueType>*>::iterator>::iterator
        iter = index_.find(key);
    if (iter == index_.end()) {
      return;
    }
    CacheNode<KeyType, ValueType>* node = *(iter->second);
    FinishErase(node);
    //从user列表中删除该对象
    nodes_.erase(iter->second);
    index_.erase(node->key);
    if (!index_.empty()) {
      //更新头节点
      ListIter begin_iter = nodes_.begin();
      index_[(*begin_iter)->key] = nodes_.begin();
    }
  }

 private:
  void Ref(CacheNode<KeyType, ValueType>* node) {
    if (node) {
      ++node->refs;
    }
  }
  void Unref(CacheNode<KeyType, ValueType>* node) {
    if (!node) {
      --node->refs;
      if (node->refs == 0) {
        destructor_(node->key, node->value);
        if (wait_erase_.count(node->key) > 0) {
          wait_erase_.erase(node->key);
        }
        delete node;
        node = nullptr;
      }
    }
  }
  void MoveToEraseContainer(CacheNode<KeyType, ValueType>* node) {
    if (wait_erase_.count(node->key) == 0) {
      wait_erase_[node->key] = node;
    }
  }
  // 这个是不带锁的版本
  void FinishErase(CacheNode<KeyType, ValueType>* node) {
    if (node) {
      node->in_cache = false;
      MoveToEraseContainer(node);
      Unref(node);
    }
  }

 private:
  const uint32_t capacity_;
  uint32_t cur_size_ = 0;
  std::list<CacheNode<KeyType, ValueType>*> nodes_;
  typename std::unordered_map<
      KeyType, typename std::list<CacheNode<KeyType, ValueType>*>::iterator>
      index_;
  std::unordered_map<KeyType, CacheNode<KeyType, ValueType>*> wait_erase_;
  std::function<void(const KeyType& key, ValueType* value)> destructor_;
};
}  // namespace corekv
