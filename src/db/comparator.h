#ifndef DB_COMPARATOR_H_
#define DB_COMPARATOR_H_
#include <stdint.h>
namespace corekv
{
    // 按照字典序列
    class ByteComparator final {
        public:
        int32_t Compare(const char* a, const char*b);
    };
} // namespace corekv

#endif