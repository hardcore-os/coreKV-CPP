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
    } // namespace util
    
} // namespace corekv


#endif