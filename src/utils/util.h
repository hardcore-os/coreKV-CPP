#ifndef UTILS_UTIL_H_
#define UTILS_UTIL_H_
#include <stdint.h>

#include <string>

namespace corekv
{
    namespace util
    {
        //单位是ms
        uint64_t GetCurrentTime();
        void GetCurrentTimeString(std::string&output);
        int64_t GetCurrentTid();
        int64_t GetCurrentPid();
        bool CheckLittleEndian();
        uint32_t DecodeFixed32(const char* ptr);
        //
        uint32_t SimMurMurHash(const char *data, uint32_t len);
        int64_t Next2Power(uint64_t value);
    } // namespace util
    
} // namespace corekv


#endif