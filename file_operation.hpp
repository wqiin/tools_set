#ifndef FILE_OPERATION_H
#define FILE_OPERATION_H

#include <filesystem>
#include <string_view>

namespace Helper{
    namespace file{

        bool is_file_exist(std::string_view strFile)
        {
            std::error_code ec;
            auto exists = std::filesystem::exists(std::filesystem::path(strFile), ec);
            return !ec && exists;
        }




    }//namespace file
}//namespace Helper

#endif // FILE_OPERATION_H
