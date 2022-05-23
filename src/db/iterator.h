#pragma once
#include <assert.h>
#include <string_view>
#include <string>
#include "status.h"

namespace corekv {

class Iterator {
 public:
  Iterator();

  Iterator(const Iterator&) = delete;
  Iterator& operator=(const Iterator&) = delete;

  virtual ~Iterator();

  virtual bool Valid() const = 0;

  virtual void SeekToFirst() = 0;

  virtual void SeekToLast() = 0;

  virtual void Seek(const std::string_view& target) = 0;

  virtual void Next() = 0;

  virtual void Prev() = 0;
  virtual std::string_view key() const = 0;
  virtual std::string value() = 0;

  virtual DBStatus status() const = 0;
  using CleanupFunction = void (*)(void* arg1, void* arg2);
  void RegisterCleanup(CleanupFunction function, void* arg1, void* arg2);

 private:
  struct CleanupNode {
    bool IsEmpty() const { return function == nullptr; }

    void Run() {
      assert(function != nullptr);
      (*function)(arg1, arg2);
    }
    CleanupFunction function;
    void* arg1;
    void* arg2;
    CleanupNode* next;
  };
  CleanupNode cleanup_head_;
};

Iterator* NewEmptyIterator();

Iterator* NewErrorIterator(const DBStatus& status);

}  // namespace corekv
