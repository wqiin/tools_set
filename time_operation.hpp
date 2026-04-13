#ifndef TIME_OPERATION_H
#define TIME_OPERATION_H

#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdint>
#include <chrono>
#include <type_traits>
#include <variant>

#include<QElapsedTimer>

namespace Helper
{
    inline static const char* DEFAULT_DATETIME_FORMAT = "%Y-%m-%d %H:%M:%S";
    inline static const char* DEFAULT_DATE_FORMAT = "%Y-%m-%d";
    inline static const char* DEFAULT_TIME_FORMAT = "%H:%M:%S";

    /*!
     *  @brief          格式化日期时间为字符串
     *  @param          [const std::tm & lcoal_time] std::localtime转换后的结构体时间
     *  @param          [const char * fmt = DEFAULT_DATETIME_FORMAT] 格式
     *  @return         [std::string]
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_format_datetime(const std::tm & lcoal_time, const char * fmt = DEFAULT_DATETIME_FORMAT)
    {
        std::ostringstream oss;
        oss << std::put_time(&lcoal_time, fmt);
        return oss.str();
    }

    /*!
     *  @brief          格式化日期为字符串
     *  @param          [const std::tm & lcoal_time] std::localtime转换后的结构体时间
     *  @param          [const char * fmt = DEFAULT_DATE_FORMAT] 格式
     *  @return         [std::string]
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_format_date(const std::tm & lcoal_time, const char * fmt = DEFAULT_DATE_FORMAT)
    {
        return get_format_datetime(lcoal_time, DEFAULT_DATE_FORMAT);
    }

    /*!
     *  @brief          格式化时间为字符串
     *  @param          [const std::tm & lcoal_time] std::localtime转换后的结构体时间
     *  @param          [const char * fmt = DEFAULT_TIME_FORMAT] 格式
     *  @return         [std::string]
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_format_time(const std::tm & lcoal_time, const char * fmt = DEFAULT_TIME_FORMAT)
    {
        return get_format_datetime(lcoal_time, DEFAULT_TIME_FORMAT);
    }

    /*!
     *  @brief          获取当前的时间
     *  @param          [null]
     *  @return         [std::tm] 标准库的日期时间结构体
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::tm get_current_datetime()
    {
        std::time_t t = std::time(nullptr);
        return *std::localtime(&t);
    }

    /*!
     *  @brief          获取当前日期时间格式化后的字符串
     *  @param          [const char * fmt = DEFAULT_DATETIME_FORMAT] 格式
     *  @return         [std::string] 日期时间格式化后的字符串
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_current_format_datetime(const char * fmt = DEFAULT_DATETIME_FORMAT)
    {
        return get_format_datetime(get_current_datetime(), fmt);
    }

    /*!
     *  @brief          获取当前日期格式化后的字符串
     *  @param          [const char * fmt = DEFAULT_DATE_FORMAT] 格式
     *  @return         [std::string] 日期格式化后的字符串
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_current_format_date(const char * fmt = DEFAULT_DATE_FORMAT)
    {
        return get_format_datetime(get_current_datetime(), fmt);
    }


    /*!
     *  @brief          获取当前时间格式化后的字符串
     *  @param          [const char * fmt = DEFAULT_TIME_FORMAT] 格式
     *  @return         [std::string] 时间格式化后的字符串
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_current_format_time(const char * fmt = DEFAULT_TIME_FORMAT)
    {
        return get_format_datetime(get_current_datetime(), fmt);
    }

    /*!
     *  @brief          获取当前时间戳，以秒的形式
     *  @param          []
     *  @return         [std::uint64_t] 从 1970-01-01（Unix Epoch） 到现在的时间(秒)
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::uint64_t get_timespamp()
    {
        using namespace std::chrono;
        auto now_s = system_clock::now();
        auto secs = duration_cast<seconds>(now_s.time_since_epoch()).count();
        return secs;

        //return std::time(nullptr);
    }

    /*!
     *  @brief          获取当前时间戳，以毫秒的形式
     *  @param          []
     *  @return         [std::uint64_t] 从 1970-01-01（Unix Epoch） 到现在的时间(毫秒)
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::uint64_t get_timespamp_as_ms()
    {
        using namespace std::chrono;
        auto now_s = system_clock::now();
        auto ms = duration_cast<milliseconds>(now_s.time_since_epoch()).count();
        return ms;
    }

    /*!
     *  @brief          获取当前时间戳（字符串形式返回）
     *  @param          []
     *  @return         [std::string] 从 1970-01-01（Unix Epoch） 到现在的时间(秒)（以字符串形式返回）
     *  @author         zys
     *  @date           2026-04-03
     */
    static std::string get_timespamp_as_string()
    {
        return std::to_string(get_timespamp());
    }

    /*!
     *  @brief          获取字符串时间戳转换为std::tm
     *  @param          [const std::string & strTimestamp] 格式化的时间（字符串形式）
     *  @param          [const char * fmt = DEFAULT_DATETIME_FORMAT] 需要转换的格式，默认为转换日期和时间
     *  @return         [std::optional<std::tm>] 转换失败返回std::nullopt
     *  @author         zys
     *  @date           2026-04-05
     */
    static std::optional<std::tm> string_to_tm(const std::string & strTimestamp, const char * fmt = DEFAULT_DATETIME_FORMAT)
    {
        std::tm local_tm;
        std::istringstream ss(strTimestamp);
        ss >> std::get_time(&local_tm, fmt);

        if (ss.fail()) {
            // Reset failbit and clear the stream for further operations if necessary
            ss.clear();
            return std::nullopt;
        }

        return local_tm;
    }

    //计时器，计算花费的时间
    class time_elapsed{
        using Clock_t = std::chrono::steady_clock;

    protected:
        Clock_t::time_point m_start_time_point;
        std::string m_name;

    public:
        time_elapsed(const std::string & name = "") : m_name(name), m_start_time_point(Clock_t::now())  {}

        //重新开始计时
        void reset() noexcept
        {
            m_start_time_point = Clock_t::now();
        }

        //返回花费的时间
        template<class time_t = std::chrono::milliseconds>
        std::uint64_t elapsed_time() const noexcept
        {
            auto end = Clock_t::now();
            auto elapsed = std::chrono::duration_cast<time_t>(end - m_start_time_point).count();

            return elapsed;
        }

        //测量传入可调用对象花费的时间 - 可变模版参数后面，不能再跟默认的模版参数
        template<class Duration_t = std::chrono::milliseconds, class Func, class ...Args>
        [[nodiscard]] static auto measure(Func && func, Args&& ... args)
        {
            //判断一个“可调用对象”能不能被调用，并且返回值能不能转换成指定类型
            static_assert(std::is_invocable_v<Func, Args...>, "func MUST be callable with given arguments.");

            using Result_t = std::invoke_result_t<Func, Args...>;//函数返回值类型，有可能是void
            using Ret_t = std::conditional_t<std::is_void_v<Result_t>, std::monostate, Result_t>;

            auto now = Clock_t::now();
            if constexpr(std::is_same_v<std::monostate, Ret_t>){
                //在模版里面调用传入的可调用对象和参数，使用完美转发？
                std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

                auto end = Clock_t::now();
                std::int64_t elapsed = std::chrono::duration_cast<Duration_t>(end - now).count();

                return std::pair<Ret_t, std::int64_t>{std::monostate{}, elapsed};
            }else{
                auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);

                auto end = Clock_t::now();
                auto elapsed = std::chrono::duration_cast<Duration_t>(end - now).count();

                return std::pair<Ret_t, std::int64_t>{std::move(result), elapsed};
            }

            //剔除const、voltaile和引用修饰符，得到纯粹的类型
            //std::is_same_v<std::remove_cvref<T>, std::remove_cvref<U>>;
        }
    };

}

#endif // TIME_OPERATION_H
