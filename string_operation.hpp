#ifndef STRING_OPERATION_H
#define STRING_OPERATION_H


#include <string>
#include <cstdarg>
#include <vector>
#include <cstdio>
#include <sstream>
#include <optional>


namespace Helper{
    namespace string{
        /*!
         *  @brief          格式化字符串
         *  @param          [const char* fmt] 字符串格式
         *  @param          [...] 参数
         *  @return         [std::string] 格式化后的字符串
         *  @author         zys
         *  @date           2026-04-03
        */
        static std::string format(const char* fmt, ...) {
            va_list args;
            va_start(args, fmt);

            // 先拷贝一份 args（因为 vsnprintf 会消耗）
            va_list args_copy;
            va_copy(args_copy, args);

            // 获取需要的长度
            int len = std::vsnprintf(nullptr, 0, fmt, args);
            va_end(args);

            if (len < 0) {
                va_end(args_copy);
                return "";
            }

            std::vector<char> buf(len + 1);
            std::vsnprintf(buf.data(), buf.size(), fmt, args_copy);
            va_end(args_copy);

            return std::string(buf.data(), len);
        }

        /*!
         *  @brief          分割字符串(会剔除空字符串)
         *  @param          [const string &]
         *  @param          [char] 分割的字符
         *  @return         [std::vector<string>] 分割后的字符串数组
         *  @author         zys
         *  @date           2026-04-03
        */
        static std::vector<std::string> split_string(const std::string &str, const char delimter)
        {
            std::vector<std::string> tokens;
            std::istringstream ss(str);
            std::string token;

            while (std::getline(ss, token, delimter)){
                if(!token.empty()){
                    tokens.push_back(token);
                }
            }

            return tokens;
        }


        static std::string join_string(const std::vector<std::string> & vec_strs, const char delimter = ' ')
        {
            if(vec_strs.empty()){
                return {};
            }

            size_t total_size = 0;
            for(const auto & str : vec_strs){
                total_size += str.size();
            }

            std::string result;
            result.reserve(total_size);

            for(auto ii = 0; ii < vec_strs.size(); ii++){
                result += vec_strs.at(ii);

                if(ii != vec_strs.size() - 1){
                    result += delimter;
                }
            }

            return result;
        }

        template<class... Args>
        static std::string join_string(const char delimter, const std::string & str_first, Args... args)
        {
            std::string result = str_first;

            if constexpr(sizeof...(args) > 0){
                result += delimter + join_string(delimter, args...);
            }

            return result;
        }

        /*!
         *  @brief          字符串替换给定的字串
         *  @param          [std::string& src] 源字符串
         *  @param          [const std::string& from] 需要被替换的子串
         *  @param          [const std::string& to] 替换后的字符串
         *  @return         [std::string] 替换后的字符串
         *  @author         zys
         *  @date           2026-04-03
        */
        static std::string replace_string(std::string& src, const std::string& from, const std::string& to)
        {
            for (size_t pos = src.find(from); pos != std::string::npos; pos = src.find(from, pos)) {
                src.replace(pos, from.length(), to);
            }
            return src;
        }

        /*!
         *  @brief          字符串转换为数字(不抛出异常版本)
         *  @param          [const std::string &str] 源字符串,包含数字
         *  @return         [Number] 转换后的数字
         *  @author         zys
         *  @date           2026-04-03
        */
        template<typename Number>
        static std::optional<Number> string_to_number(const std::string & str)
        {
            Number num;
            std::stringstream ss(str);
            ss >> num;

            return (ss.eof() || ss.good()) ? num : std::nullopt;
        }


    }
}

#endif // STRING_OPERATION_H
