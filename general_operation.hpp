
#ifndef GENERAL_OPERATION_HPP
#define GENERAL_OPERATION_HPP

#include <memory>
#include <thread>
#include <type_traits>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <functional>
#include <map>
#include <shared_mutex>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <ranges>

#include <flat_map>

#include <iostream>


#include "general_define.hpp"

namespace Helper{
    namespace general{
        using namespace Helper::emun_definition;

        template<typename T>
        static std::shared_ptr<T> copy_as_ptr(const T & obj)
        {
            return std::make_shared<T>(obj);
        }

        //定义容器模版要求的基础模版类型
        template <typename T>
        concept ContainerLike  = requires(T a){
            a.begin();
            a.end();
            a.empty();
            a.clear();
            a.insert();
        };

        template<ContainerLike container_t>
        static void swap_append(container_t & dst, container_t & from_append)
        {
            if(from_append.empty()){
                return ;
            }

            dst.insert(dst.end(), std::make_move_iterator(from_append.begin()), std::make_move_iterator(from_append.end()));
            from_append.clear();
        }


        static size_t get_current_thread_id()
        {
            std::hash<std::thread::id> hasher;
            return hasher(std::this_thread::get_id());
        }

        static constexpr std::uint16_t make_uint(const std::uint8_t h, const std::uint8_t l)
        {
            std::uint16_t result = h;
            result = result << 8 | l;

            return result;
        }

        static constexpr std::uint32_t make_uint(const std::uint16_t h, const std::uint16_t l)
        {
            std::uint32_t result = h;
            result = result << 16 | l;

            return result;
        }

        static constexpr std::uint64_t make_uint(const std::uint32_t h, const std::uint32_t l)
        {
            std::uint64_t result = h;
            result = result << 32 | l;

            return result;
        }

        template<typename Result, typename T>
        static constexpr Result make_uint(const T h, const T l)
        {
            static_assert(std::is_unsigned_v<T>, "T must be unsigned");
            static_assert(std::is_unsigned_v<Result>, "Result must be unsigned");

            // Result 必须能装下两个 T
            static_assert(sizeof(Result) >= sizeof(T) * 2,
                          "Result is too small to hold combined value");

            constexpr size_t shift = sizeof(T) * 8;

            return (static_cast<Result>(h) << shift) |
                   static_cast<Result>(l);
        }

        template<typename T>
        inline static constexpr std::uint8_t get_high8(const T num)
        {
            static_assert(std::is_unsigned_v<T>, "T must be unsigned");

            constexpr size_t shift = (sizeof(T) - 1) * 8;

            return static_cast<std::uint8_t>(num >> shift);
        }

        template<typename T>
        inline static constexpr std::uint8_t get_low8(const T num)
        {
            static_assert(std::is_unsigned_v<T>, "T must be unsigned");

            return static_cast<std::uint8_t>(num & 0xFF);
        }


        /*
            std::addressof的源码大概如下
            operator& 只对原类型生效，std::addressof 的本质是：通过把对象“伪装成 char”，绕过 operator&，获取真实地址
                template <class T>
                constexpr T* addressof(T& arg) noexcept
            {
                return reinterpret_cast<T*>(
                    &const_cast<char&>(
                        reinterpret_cast<const volatile char&>(arg)
                        )
                    );
            }
        */
        template<typename container_t, typename Pred>
        static auto find_if_as_ptr(container_t & c, Pred && p)
        {
            using std::begin;
            using std::end;

            auto it = std::find_if(begin(c), end(c), std::forward<Pred>(p));

            if (it == end(c))
                return static_cast<std::remove_reference_t<decltype(*it)>*>(nullptr);

            //C++ 允许你重载 operator&； std::addressof 解决什么问题？ 一定拿到真实内存地址，不会被重载影响
            return std::addressof(*it);
        }

        template<typename T>
        static void safe_delete(T* & p)
        {
            if(p){
                delete p;
                p = nullptr;
            }
        }


        //本质 - 安全调用函数对象，如果为空就跳过或返回默认值， 常见于 - 回调系统（callback）、事件系统（event、插件系统 、可选行为（hook）
        template<typename func_t, typename... args_t>
        static auto safe_calling(func_t && f, args_t && ... args)
        {
            using return_t = std::invoke_result_t<func_t, args_t...>;

            if constexpr(std::is_void_v<return_t>){
                if constexpr(requires {static_cast<bool>(f);}){
                    return ;
                }

                std::invoke(std::forward<func_t>(f), std::forward<args_t>(args)...);
            }else{
                if constexpr(requires {static_cast<bool>(f);}){
                    return ;
                }

                return std::invoke(std::forward<func_t>(f), std::forward<args_t>(args)...);
            }

            //老式版本写法
            //using return_t  = decltype(std::forward<func_t>(f)(std::forward<args_t>(args)...));
            // if constexpr(std::is_same_v<return_t, void>){
            //     std::forward<func_t>(f)(std::forward<args_t>(args)...);
            // }else{
                    //对于lambda，其不支持bool判断
            //     return f ? std::forward<func_t>(f)(std::forward<args_t>(args)...) : return_t{};
            // }
        }

        //你这个类本质是：“在作用域结束时自动执行代码”的工具（RAII / Scope Guard）
        template<typename func_t>
        class scope_guard{
            public:
            static inline int nIndex = 0;//用以验证是否执行了C++17的原地构造行为的计数器
                explicit scope_guard(func_t f) noexcept : m_f(std::move(f)), m_bIsActive(true) {
                    nIndex += 1;
                    std::cout << "default times: " << nIndex << std::endl;
                }

                //只有当 F 的移动构造不抛异常时，这个函数才是 noexcept
                scope_guard(scope_guard && other) noexcept(std::is_nothrow_move_constructible_v<func_t>) : m_f(std::move(other.m_f)), m_bIsActive(other.m_bIsActive){
                    other.m_bIsActive = false;

                    nIndex += 1;
                    std::cout << "move times: " << nIndex << std::endl;
                }

                //析构函数绝对不应该抛异常
                ~scope_guard() noexcept{
                    if(m_bIsActive){
                        m_f();
                    }
                }

                //这个函数保证不会抛异常（no exception）
                //如果“说了不抛，但实际抛了”会怎样？ - 直接终止程序：std::terminate() 💥
                //为什么要用 noexcept？ -
                //1. 提升性能（非常重要）；
                //2. 提供“强异常安全保证” - 告诉编译器 & 使用者：这个函数不会失败
                //3. 参与模板选择（SFINAE / concept）std::is_nothrow_move_constructible<T> 👉 STL 会用这个判断
                //4. 编译器优化 👉 可以省去：异常栈展开、一些检查逻辑
                void dismiss() noexcept{
                    this->m_bIsActive = false;
                }

                scope_guard(const scope_guard & ) = delete;
                scope_guard & operator=(const scope_guard &) = delete;
                scope_guard & operator=(scope_guard &&) = delete;

            private:
                func_t m_f;
                bool m_bIsActive;
        };


        template<class Ret, class K, class ...Args>
        class function_map
        {
        public:
            using Functor = std::function<Ret(Args...)>;

            void register_func(const K & key, Functor func) noexcept
            {
                std::unique_lock lock(m_mtx);
                m_mpK2Func.try_emplace(key, std::move(func));
            }

            const Functor* get_functor(const K & key) const noexcept
            {
                std::shared_lock lock(m_mtx);
                auto it = m_mpK2Func.find(key);
                if(it != m_mpK2Func.end()){
                    return &(it->second);
                }

                return nullptr;
            }

            Functor* get_functor(const K & key) noexcept
            {
                std::shared_lock lock(m_mtx);
                auto it = m_mpK2Func.find(key);
                if(it != m_mpK2Func.end()){
                    return &(it->second);
                }

                return nullptr;
            }

            bool contains(const K & key) const noexcept
            {
                std::shared_lock lock(m_mtx);
                return m_mpK2Func.contains(key);
            }

            std::optional<Ret> run_functor(const K & key, Args... args) const
            {
                if(auto fx = get_functor(key)){
                    return (*fx)(std::forward<Args>(args)...);
                }

                return std::nullopt;
            }

        protected:
            mutable std::shared_mutex m_mtx;
            std::map<K, Functor> m_mpK2Func;
        };


        //可作为轻量级的锁，防止重复执行
        template<class T>
        class atomic_guard
        {
        public:
            //[[__nodiscard__]]  用于提醒，“你创建的锁对象被丢弃了，可能是错误”
            [[__nodiscard__]] atomic_guard(std::atomic<T> & src, const T & cur_expected_valud) : m_src(src), m_currentExpectedValue(cur_expected_valud)
            {}

            ~atomic_guard()
            {
                if(m_bRecovery){
                    m_src.store(m_currentExpectedValue);
                }
            }

            bool try_store(const T & expected_will)
            {
                T cur_value = this->m_currentExpectedValue;

                //如果m_src不等于cur_value时，则cur_value会被赋值为m_src的当前值，返回false；如果相等则不会被修改，即m_src会被修改为expected_will,返回true
                m_bRecovery = m_src.compare_exchange_strong(cur_value, expected_will);

                return m_bRecovery;
            }

        protected:
            bool m_bRecovery = false;       //是否在析构是将原子对象的值还原
            std::atomic<T> & m_src;         //原子对象引用
            T m_currentExpectedValue;       //期望的当前值
        };


        class sync_waiter
        {
        public:
            using Clock = std::chrono::steady_clock;

        public:
            sync_waiter() = default;
            ~sync_waiter() = default;

            sync_waiter(const sync_waiter &) = delete;
            sync_waiter(sync_waiter &&) = delete;

            sync_waiter & operator=(const sync_waiter &) = delete;
            sync_waiter & operator=(sync_waiter &&) = delete;

            //主动中断所有等待
            void interrupt() noexcept
            {
                m_interrupted.store(true, std::memory_order_release);

                std::lock_guard<std::mutex> lock(m_mtx);
                m_cv.notify_all();
            }

            [[nodiscard ("Return MUST not be ignored")]] bool is_interrupted() const noexcept
            {
                return m_interrupted.load(std::memory_order_acquire);
            }

            [[nodiscard ("Return MUST not be ignored")]] bool is_waitting() const noexcept
            {
                return m_waiiting.load(std::memory_order_acquire);
            }

            void reset() noexcept
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                m_interrupted.store(false, std::memory_order_relaxed);
            }

            /**
             * @brief 同步等待直到条件为真，或超时/中断
             * @tparam Pred 类型自动推导，支持 lambda
             * @param pred 条件函数，返回 true 表示成功
             * @param interval_ms 轮询间隔
             * @param timeout_ms 总超时时间
             * @return ENWaiterStatus 状态码，标识该方法执行结果
             */
            template<class Pred>
            ENWaiterStatus wait_until(Pred && pred, std::uint32_t timeout_ms, std::uint32_t interval_ms = 100)
            {
                //判断一个“可调用对象”能不能被调用，并且返回值能不能转换成指定类型
                static_assert(std::is_invocable_r_v<bool, Pred>, "Pred MUST be callable and reutrn bool.");

                if(m_interrupted.load(std::memory_order_acquire)){
                    return ENWaiterStatus::En_invalid;
                }

                bool bExpected = false;
                if(!m_waiiting.compare_exchange_strong(bExpected, true)){
                    return ENWaiterStatus::En_invalid;
                }


                scope_guard guard([this]{
                    m_waiiting = false;
                });

                const auto start_time = Clock::now();
                const std::chrono::milliseconds interval(interval_ms);
                const std::chrono::milliseconds timeout(timeout_ms);

                while(true){
                    //检查是否被中断，- 可能在pred执行时被外部中断
                    if(m_interrupted.load(std::memory_order_acquire)){
                        return ENWaiterStatus::En_interrupt;
                    }

                    //检查条件是否满足
                    if(pred()){
                        return ENWaiterStatus::En_success;
                    }

                    //检查是否超时
                    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now() - start_time);
                    if(elapsed >= timeout){
                        return ENWaiterStatus::En_timeout;
                    }

                    auto sleep_duration = interval;
                    if(sleep_duration > (timeout - elapsed)){
                        sleep_duration = timeout - elapsed;
                    }

                    //加锁等待超时或者被中断
                    std::unique_lock<std::mutex> lock(m_mtx);
                    m_cv.wait_for(lock, sleep_duration, [this]{
                        return m_interrupted.load(std::memory_order_acquire);
                    });

                    //当前循环轮结束，进行下一轮检查
                }//while
            }

        private:
            mutable std::mutex m_mtx;
            std::condition_variable m_cv;
            std::atomic_bool m_interrupted{false};
            std::atomic_bool m_waiiting{false};
        };

        //不断执行一个任务（todo），直到成功或被终止，中间用条件变量避免死循环占CPU
        class waiter
        {
        protected:
            std::mutex m_mtx;
            std::condition_variable m_cv;
            std::atomic_bool m_running {false};
            bool m_terminated = false;//终止标识
            std::uint32_t m_interval_ms = 0;//遍历的间隔(单位:毫秒)
            std::chrono::time_point<std::chrono::steady_clock> m_tpStart;

        public:
            void terminate()
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_terminated = true;
                m_cv.notify_all();
            }

            bool is_terminated() noexcept
            {
                std::unique_lock<std::mutex> l(m_mtx);
                return m_terminated;
            }

            template<class todo_t>
            ENWaiterStatus wait_loop(todo_t && todo, std::uint32_t interval_ms = 200)
            {
                if(is_terminated()){
                    return ENWaiterStatus::En_invalid;
                }

                {
                    std::unique_lock<std::mutex> l(m_mtx);
                    m_interval_ms = interval_ms;
                }

                using namespace std::chrono_literals;
                while(m_running){
                    if(todo()){
                        break;
                    }else{
                        std::unique_lock<std::mutex> l(m_mtx);
                        //条件成立，返回truel;超时或者被唤醒时条件不满足，则返回false
                        if(m_cv.wait_for(l, std::chrono::milliseconds(m_interval_ms), [this]{
                            return m_terminated;
                        }))
                        {
                            break;
                        }

                    }
                }
            }
        };


        //线程安全的 key-value 缓存 + 支持等待某个条件成立（可中断/超时）”
        template<class K, class V>
        class sync_map
        {
        protected:
            std::condition_variable_any m_cv;
            mutable std::recursive_mutex m_mtx;
            std::map<K, V> m_k2v;
            std::atomic_uint64_t m_waitVersion{0};

        public:
            std::optional<V> value(const K & k) const noexcept
            {
                std::unique_lock<std::recursive_mutex> l(m_mtx);
                auto it = m_k2v.find(k);
                if(it != m_k2v.end()){
                    return it->second;
                }

                return std::nullopt;
            }

            std::optional<V> value_no_lock(const K & k) const noexcept
            {
                auto it = m_k2v.find(k);
                if(it != m_k2v.end()){
                    return it->second;
                }

                return std::nullopt;
            }

            std::optional<V> operator[](const K & k) const noexcept
            {
                return value_no_lock(k);
            }

            void set_value(const K & k, const V & v) noexcept
            {
                std::unique_lock<std::recursive_mutex> l(m_mtx);
                m_k2v[k] = v;

                m_cv.notify_all();
            }

            void set_by_map(const std::map<K, V> & mp) noexcept
            {
                if(mp.empty()){
                    return ;
                }

                std::unique_lock<std::recursive_mutex> l(m_mtx);
                for(const auto & [k, v] : mp){
                    m_k2v[k] = v;
                }

                m_cv.notify_all();
            }

            void remove(const K & k) noexcept
            {
                std::unique_lock<std::recursive_mutex> l(m_mtx);
                auto removed = m_k2v.erase(k);
                m_cv.notify_all();

                return removed;
            }

            void inerrupt_all() noexcept
            {
                m_waitVersion.fetch_add(1);
                m_cv.notify_all();
            }

            template<class func_t>
            void for_each_lock(func_t && func) noexcept
            {
                int updated_cnt = 0;

                {
                    std::unique_lock<std::recursive_mutex> l(m_mtx);
                    for(auto & kv : m_k2v){
                        if(func(kv.first, kv.second)){
                            updated_cnt += 1;
                        }
                    }
                }

                if(updated_cnt > 0){
                    m_cv.notify_all();
                }
            }

            template<class Pred>
            ENWaiterStatus wait_for(std::uint32_t timeout_ms, const K & k, Pred && pred)
            {
                //判断是否存在该值
                auto value_opt = value(k);
                if(value_opt && pred(*value_opt)){
                    return ENWaiterStatus::En_success;
                }

                //不满足就等待
                return wait_for_cache(timeout_ms, [&k, pred](const auto & ref_this){
                    if(auto v_opt = ref_this[k]){
                        return pred(*v_opt);
                    }

                    return false;
                });
            }

            template<class Pred>
            ENWaiterStatus wait_for_cache(std::uint32_t timeout_ms, Pred && pred)
            {
                auto wait_ver = m_waitVersion.load();
                bool bInterrupted = false;

                std::unique_lock<std::recursive_mutex> l(m_mtx);

                //wait_for不同的重载，返回值不一样；带谓词的返回bool,不带谓词的返回枚举
                bool timeout = m_cv.wait_for(l, std::chrono::milliseconds(timeout_ms),
                                            [p = std::forward<Pred>(pred), this, wait_ver, &bInterrupted]{
                                                bInterrupted = wait_ver < m_waitVersion.load();
                                                return bInterrupted || p(*this);
                                            });

                if(!timeout){
                    return ENWaiterStatus::En_timeout;
                }else{
                    return bInterrupted ? ENWaiterStatus::En_interrupt : ENWaiterStatus::En_success;
                }
            }
        };

        template<class View, class Locker>
        class locked_view{
        public:
            locked_view(View & v, Locker & l) : m_view(std::move(v)), m_locker(std::move(l)){}

            //用以支持for-ranged遍历方式
            auto begin(){
                return m_view.begin();
            }

            auto end(){
                return m_view.end();
            }

            auto begin() const{
                return m_view.begin();
            }

            auto end() const{
                return m_view.end();
            }

        private:
            View m_view;
            Locker m_locker;
        };

        //信号量，默认为二进制信号量
        class semaphore
        {
        protected:
            std::mutex m_mtx;
            std::condition_variable m_cv;
            int m_count;
            const int m_initial_count = 0;//存放初始值，一旦初始化，不能被修改

        public:
            //默认为二进制的信号量
            semaphore(int nCount = 1):m_initial_count(nCount), m_count(nCount){}

            inline void post()
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                m_count += 1;

                m_cv.notify_one();
            }

            inline void wait()
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                m_cv.wait(lock, [this]{
                    return m_count > 0;
                });

                m_count -= 1;
            }

            inline bool timed_wait_for(const std::uint32_t timeout_ms)
            {
                std::unique_lock<std::mutex> lock(m_mtx);
                auto bRet = m_cv.wait_for(lock, std::chrono::milliseconds(timeout_ms), [this]{
                    return m_count > 0;
                });

                if(bRet){
                    m_count -= 1;
                }

                return bRet;
            }

            void reset(const int new_count)
            {
                std::lock_guard<std::mutex> lock(m_mtx);
                m_count = new_count;

                if(m_count > 0){
                    m_cv.notify_all();//唤醒所有等待资源的线程
                }else if(m_count < 0){
                    m_count = 0;//资源为0，为错误，可以考虑抛出异常
                }
            }

            void reset()
            {
                reset(m_initial_count);
            }

        };

    }//namespace general
}//namespace Helper


#endif // GENERAL_OPERATION_HPP
