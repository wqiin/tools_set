#ifndef FILESYSTEM_OPERATION_H
#define FILESYSTEM_OPERATION_H

#include <filesystem>
#include <string_view>
#include <cstdint>
#include <system_error>

#include <optional>


namespace Helper{
    namespace fs{

        namespace fs = std::filesystem;

        static bool is_file_exist(std::string_view strFile, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto exists = std::filesystem::exists(std::filesystem::path(strFile), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
            }

            return !ec && exists;
        }


        static std::int64_t file_size(std::string_view file, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            std::uintmax_t size = fs::file_size(fs::path(file), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec ? size : -1;
        }



        //删除文件或者空目录
        static bool remove_file(std::string_view file, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto removed = fs::remove(fs::path(file), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && removed;
        }

        static bool remove_directories(std::string_view file, std::string * err_msg = nullptr) noexcept
        {
            //remove_all会递归删除，需要安全校验，请谨慎使用
            std::error_code ec;
            auto removed = fs::remove_all(fs::path(file), ec);//成功删除的文件和目录数量
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && removed;
        }

        static bool copy(std::string_view from, std::string_view to, fs::copy_options option = fs::copy_options::overwrite_existing, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto copied = fs::copy_file(fs::path(from), fs::path(to), option, ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && copied;
        }

        //默认覆盖已经存在的文件
        static bool copy_file(std::string_view from, std::string_view to, fs::copy_options option = fs::copy_options::overwrite_existing, std::string * err_msg = nullptr) noexcept
        {
            return copy(from, to, option, err_msg);
        }


        //默认递归复制原目录到目的目录
        static bool copy_directory(std::string_view from, std::string_view to, fs::copy_options option = fs::copy_options::recursive, std::string * err_msg = nullptr) noexcept
        {
            return copy(from, to, option, err_msg);
        }

        //重命名或者移动
        static bool rename(std::string_view from, std::string_view to, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            fs::rename(from, to, ec);

            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec;
        }


        static bool rename_file(std::string_view from, std::string_view to, std::string * err_msg = nullptr) noexcept
        {
            return rename(from, to, err_msg);
        }

        static bool rename_directory(std::string_view from, std::string_view to, std::string * err_msg = nullptr) noexcept
        {
            return rename(from, to, err_msg);
        }


        static bool create_directory(std::string_view path, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto created = fs::create_directory(fs::path(path), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && created;
        }


        static bool create_directories(std::string_view path, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto created = fs::create_directories(fs::path(path), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && created;
        }


        static std::string absolute_path(std::string_view path, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;

            //需要确保path存在，否则调用非ec版本的时候，会抛出异常
            auto ret_path = fs::canonical(path, ec);
            if(ec && err_msg){
                *err_msg = ec.message();
            }

            //fs::absolute(fs::path("/hello/world"));//当前程序的路径 + 相对路径,得到/Users/hola + /hello/world
            //不会解析 . / .. / 符号链接
            //fs::absolute 只是把路径“补成绝对路径”，不解析、不检查、不访问文件系统

            return ret_path.string();
        }


    }//namespace file
}//namespace Helper




#endif // FILESYSTEM_OPERATION_H
