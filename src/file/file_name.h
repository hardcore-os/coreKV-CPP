#pragma once
#include <string>
namespace corekv
{
    class FileName {
        public:
        static std::string DescriptorFileName(const std::string& dbname, const std::string&file_name);
        static std::string FileNameSSTable(const std::string& dbname, uint64_t sst_id);
        
    };
} // namespace corekv
