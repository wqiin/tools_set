#ifndef FILESYSTEM_OPERATION_H
#define FILESYSTEM_OPERATION_H

#include <filesystem>
#include <string_view>
#include <cstdint>
#include <system_error>

// #include <optional>
// #include <expected>


namespace Helper{
    namespace fs{

        namespace fs = std::filesystem;

        /*!
         *  @brief          判断文件是存在
         *  @param          [std::string_view strFile]    文件名（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 文件是否存在
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool file_exists(std::string_view strFile, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto exists = std::filesystem::exists(std::filesystem::path(strFile), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
            }

            return !ec && exists;
        }


        /*!
         *  @brief          获取文件的大小，单位：字节
         *  @param          [std::string_view file]    文件名（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [std::int64_t] 文件的大小，出错时返回-1
         *  @author         zys
         *  @date           2026-04-14
        */
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


        /*!
         *  @brief          删除指定的文件
         *  @param          [std::string_view file]    文件名（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功删除，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool remove_file(std::string_view file, std::string * err_msg = nullptr) noexcept
        {
            std::error_code ec;
            auto removed = fs::remove(fs::path(file), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
                //ec.category();//error source
            }

            return !ec && removed;
        }


        /*!
         *  @brief          删除指定目录下的文件和目录
         *  @param          [std::string_view directory]    目录名（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           [该方法会递归删除该路径下的所有文件和目录，请确认删除的目录]
         *  @return         [bool] 是否成功删除，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool remove_directories(std::string_view directory, std::string * err_msg = nullptr) noexcept
        {
            //remove_all会递归删除，需要安全校验，请谨慎使用
            std::error_code ec;
            auto removed = fs::remove_all(fs::path(directory), ec);//成功删除的文件和目录数量
            if(ec && err_msg){
                *err_msg = ec.message();
                ec.category();//error source
            }

            return !ec && removed;
        }

        /*!
         *  @brief          拷贝指定的文件或者目录
         *  @param          [std::string_view from]    源目录或者文件（包含路径）
         *  @param          [std::string_view to]    目的目录或者文件（包含路径）
         *  @param          [ fs::copy_options option = fs::copy_options::overwrite_existing]    拷贝时的行为，默认为如果目的文件或者目录存在，则直接覆盖
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功拷贝，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
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

        /*!
         *  @brief          拷贝指定的文件到目的文件
         *  @param          [std::string_view from]    源文件（包含路径）
         *  @param          [std::string_view to]    目的文件（包含路径）
         *  @param          [ fs::copy_options option = fs::copy_options::overwrite_existing]    拷贝时的行为，默认为如果目的文件存在，则直接覆盖
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功拷贝，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool copy_file(std::string_view from, std::string_view to, fs::copy_options option = fs::copy_options::overwrite_existing, std::string * err_msg = nullptr) noexcept
        {
            return copy(from, to, option, err_msg);
        }


        /*!
         *  @brief          拷贝指定的文件目录到目的目录
         *  @param          [std::string_view from]    源目录（包含路径）
         *  @param          [std::string_view to]    目的目录（包含路径）
         *  @param          [ fs::copy_options option = fs::copy_options::overwrite_existing]    拷贝时的行为，默认为递归拷贝源目录以及其所有子目录下的所有文件和目录
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功拷贝，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool copy_directory(std::string_view from, std::string_view to, fs::copy_options option = fs::copy_options::recursive, std::string * err_msg = nullptr) noexcept
        {
            return copy(from, to, option, err_msg);
        }


        /*!
         *  @brief          重命名或者移动文件或者目录
         *  @param          [std::string_view from]    源目录或者文件（包含路径）
         *  @param          [std::string_view to]    目的目录或者文件（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功移动或者重命名，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
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


        /*!
         *  @brief          重命名或者移动文件
         *  @param          [std::string_view from]    源文件（包含路径）
         *  @param          [std::string_view to]    目的文件（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功移动或者重命名，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool rename_file(std::string_view from, std::string_view to, std::string * err_msg = nullptr) noexcept
        {
            return rename(from, to, err_msg);
        }


        /*!
         *  @brief          重命名或者移动目录
         *  @param          [std::string_view from]    源目录（包含路径）
         *  @param          [std::string_view to]    目的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功移动或者重命名，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool rename_directory(std::string_view from, std::string_view to, std::string * err_msg = nullptr) noexcept
        {
            return rename(from, to, err_msg);
        }


        /*!
         *  @brief          创建指定的单级目录
         *  @param          [std::string_view path]    需要创建的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           [前提是，需要确保path的各级父目录都存在]
         *  @return         [bool] 是否成功创建目录，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
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


        /*!
         *  @brief          递归创建指定的目录
         *  @param          [std::string_view path]    需要创建的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功创建目录，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
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


        /*!
         *  @brief          将传入的相对路径转换为当前系统的绝对路径
         *  @param          [std::string_view path]    需要创建的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [std::string] 转换后的绝对路径
         *  @author         zys
         *  @date           2026-04-14
        */
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


        /*!
         *  @brief          获取当前程序的工作目录
         *  @note           []
         *  @return         [std::string] 当前程序的工作目录
         *  @author         zys
         *  @date           2026-04-14
        */
        static std::string get_current_path()
        {
            return fs::current_path().string();
        }


        /*!
         *  @brief          设置当前程序的运行路径
         *  @param          [std::string_view path]    需要创建的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 是否成功设置，失败返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool set_current_path(std::string_view path, std::string * err_msg = nullptr)
        {
            std::error_code ec;
            fs::current_path(fs::path(path), ec);
            if(ec && err_msg){
                *err_msg = ec.message();
            }

            return !ec;
        }


        /*!
         *  @brief          确保指定的目录存在，如果不存在则创建
         *  @param          [std::string_view path]    需要创建的目录（包含路径）
         *  @param          [std::string * err_msg = nullptr]    返回的错误信息，如果需要的话
         *  @note           []
         *  @return         [bool] 调用该函数后，指定的目录是否存在，不存在或者出现错误返回false
         *  @author         zys
         *  @date           2026-04-14
        */
        static bool ensure_directory(std::string_view path, std::string * err_msg = nullptr)
        {
            if(path.empty()){
                if(err_msg){
                    *err_msg = "empty being path.";
                }

                return false;
            }

            std::error_code ec;
            if(fs::exists(fs::path(path), ec)){
                if(ec){
                    if(err_msg){
                        *err_msg = ec.message();
                    }

                    return false;
                }

                if(!fs::is_directory(fs::path(path), ec)){
                    if(err_msg){
                        *err_msg = "path exists already, but NOT being a directory.";
                    }

                    return false;
                }

                return true;
            }

            if(!fs::create_directories(fs::path(path), ec)){
                if(ec){
                    if(err_msg){
                        *err_msg = ec.message();
                    }

                    return false;
                }
            }

            return true;
        }

    }//namespace file
}//namespace Helper




#endif // FILESYSTEM_OPERATION_H
