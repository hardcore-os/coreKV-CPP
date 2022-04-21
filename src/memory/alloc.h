#ifndef MEMORY_ALLOC_H_
#define MEMORY_ALLOC_H_
#include <cstdint>
#include <atomic>
namespace corekv {
/*根据FreeList设计原理，分析如下：
1.当使用next时，表示当前内存并未使用
2.当使用data时，表示该内存已经不在freelist中
因此两者不会同时使用，借助union可以节省8个字节(64bit下指针占8个字节)
*/
union FreeList {
    union FreeList* next;
    char data[1];
};

class SimpleFreeListAlloc final {
  public:
  SimpleFreeListAlloc() = default;
  ~SimpleFreeListAlloc();
    void* Allocate(int32_t n);            //分配内存
    void Deallocate(void* p, int32_t n);  //释放内存
    void* Reallocate(void* p, int32_t old_size, int32_t new_size);  //扩容
    uint32_t MemoryUsage() const {
      return memory_usage_.load(std::memory_order_relaxed);
    }

  private:
    int32_t M_Align(int32_t bytes);          //字节对齐
    int32_t M_Roundup(int32_t bytes);        //向上取整
    int32_t M_FreelistIndex(int32_t bytes);  //计算位于哪个freelist
    void* M_Refill(int32_t n);
    char* M_ChunkAlloc(int32_t size, int32_t& obj);

  private:
    static constexpr uint32_t kAlignBytes = 8;           //按照8来对齐
    static constexpr uint32_t kSmallObjectBytes = 4096;  //小对象最大的字节数据
    static constexpr uint32_t kFreeListMaxNum = kSmallObjectBytes / kAlignBytes;
    //当前可用内存起点
    char* free_list_start_pos_ = nullptr;
    //当前可用内存终点
    char* free_list_end_pos_ = nullptr;
    //总的内存大小，可以理解为bias
    int32_t heap_size_ = 0;
    FreeList* freelist_[kFreeListMaxNum] = {nullptr};
    //用户获取当前内存分配量
    std::atomic<uint32_t> memory_usage_;
};
}  // namespace corekv

#endif