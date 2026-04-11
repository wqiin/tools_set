#ifndef STD_MAP_EX_HPP
#define STD_MAP_EX_HPP

#include <map>
#include <mutex>
#include <vector>
#include <condition_variable>

#include "general_define.hpp"

namespace Helper
{
    namespace map{

    using Helper::emun_definition::ENWaiterStatus;
    //只要你的类提供 lock() 和 unlock()，就可以被 std::lock_guard 使用
    struct NonMutex{
        void lock(){};
        void unlock(){};
        void try_lock(){}
    };


    //带默认值的类型模板参数
    //模板模板参数（template template parameter）意思是： 参数本身是一个“类模板”，而不是普通类型
    //因为在MpContainer作为一个没有实例化的模版，且需要依赖K和V这两个类型参数
    template<class K, class V, class Mtx_t = NonMutex, template<class, class> class MpContainer = std::map>
    class MapEx : public MpContainer<K, V>
    {
        using BaseMp= MpContainer<K, V>;

    public:
        //把基类的构造函数“继承”过来，是 C++ 里继承标准容器时非常关键的语法；用于解决 - 继承类不能自动继承构造函数的问题
        //C++不会自动查找依赖基类中的名字
        using MpContainer<K, V>::MpContainer;
        using BaseMp::operator = ;
        using BaseMp::end;
        using BaseMp::begin;
        using BaseMp::size;
        using BaseMp::empty;
        //using BaseMp::iterator;
        using BaseMp::erase;
        using BaseMp::try_emplace;
        using BaseMp::find;

        using MtxLocker = std::lock_guard<Mtx_t>;

    public:
        template<typename Foreach_t>
        void for_each(Foreach_t && todo)
        {
            MtxLocker lock(m_mtx);
            for(auto & [k, v] : *this){
                todo(k, v);
            }
        }

        //获取Key对应的Value
        V get_value(const K & key, const V & def_val = {}) const noexcept
        {
            MtxLocker lock(m_mtx);

            auto it = this->find(key);
            if(it != this->end()){
                return it->second;
            }

            return def_val;
        }

        V take_value(const K & key, const V & def_val = {}) noexcept
        {
            MtxLocker lock(m_mtx);

            auto it = this->find(key);
            if(it != this->end()){
                auto ret_val = it->second;
                this->erase(it);
                return ret_val;
            }

            return def_val;
        }

        bool contains(const K & key)
        {
            MtxLocker l(m_mtx);
            return this->find(key) != this->end();
        }

        V * get_value_as_pointer(const K & key)
        {
            auto & self = *this;
            auto it = this->find(key);
            if(it != this->end()){
                return &(self[key]);
            }

            return nullptr;
        }

        K get_key(const V & val, const K & def_key = {}) const noexcept
        {
            MtxLocker l(m_mtx);
            for(const auto &[k, v] : *this){
                if(v == val){
                    return k;
                }
            }

            return def_key;
        }

        std::vector<K> get_keys() const
        {
            MtxLocker l(m_mtx);
            std::vector<K> keys{this->size()};
            for(const auto & [k, _] : *this){
                keys.emplace_back(k);
            }

            return keys;
        }

        //获取到mp的keys视图 - 令拷贝的keys获取接口
        // auto keys_view() const
        // {
        //     MtxLocker l(m_mtx);

        //     //等同与std::views::keys(*this);
        //     auto view = *this | std::views::keys;

        //     return locked_view(view, l);
        // }

        //签名不能写成template<class Container> Container<K> get_keys() const， 这里的Container是一个具体化的类型，而不是模板，
        //模板的模板参数
        template<template<class> class Container>
        Container<K> get_keys() const
        {
            using container_t = Container<K>;
            static_assert(
                requires(container_t c, K k){c.emplace_back(k);} ||
                    requires(container_t c, K k){c.insert(k);},
                "Container MUST support function 'emplace_back' or 'insert'.");

            container_t keys;
            MtxLocker l(m_mtx);
            if constexpr(requires(container_t c){c.reserve(this->size());}){
                keys.reserve(this->size());
            }

            //key_view就加上了锁，因而方法内不需要加锁
            for(auto && [key, _] : *this){
                if constexpr(requires(container_t c, K v){c.emplace_back(v);}){
                    keys.emplace_back(key);//优先用 emplace_back,比 push_back 更泛型，在尾部构造依赖size
                }else{
                    keys.insert(key);
                }
            }

            return keys;
        }

        bool insert_value(const K & key, const V & val)
        {
            MtxLocker l(m_mtx);
            auto [it, inserted] = this->try_emplace(key, val);
            return inserted;
        }

        bool remove_value(const K & key)
        {
            MtxLocker l(m_mtx);

            //erase返回删除元素的个数
            return this->erase(key) > 0;
        }

    protected:
        mutable Mtx_t m_mtx;
    };


    //可等待的map
    template<class K, class V, template<class, class, class> class BaseMap = MapEx>
    class waitable_map : public BaseMap<K, V, std::mutex>
    {
    protected:
        std::condition_variable m_cv;//等待条件变量   必须用mutex 同一线程多次加锁需要用recursive_mutex + condition_variable_any
        std::atomic_uint64_t m_waitVersion{0};//等待版本号，用于中断

    public:
        using BaseMap<K, V, std::mutex>::BaseMap;
        using IsNotifyAll = bool;

        void interrupt_all() noexcept
        {
            m_waitVersion.fetch_add(1);
            m_cv.notify_all();
        }

        void notify_waiter(IsNotifyAll b = true) noexcept
        {
            b ? m_cv.notify_all() : m_cv.notify_one();
        }

        template<class pred_t>
        ENWaiterStatus wait_for(const int timeout_ms, const K & key, pred_t && pred) const
        {
            bool interrupt = false;
            auto cur_wait_ver = m_waitVersion.load();
            std::unique_lock<std::mutex> lock(this->m_mtx);

            using namespace std::chrono;
            auto waited = m_cv.wait_for(lock, milliseconds(timeout_ms), [this, cv_pred = std::forward<pred_t>(pred), &interrupt, cur_wait_ver, &key]{
                interrupt = cur_wait_ver < m_waitVersion.load();
                if(interrupt){
                    return emun_definition::ENWaiterStatus::En_interrupt;
                }

                if(auto it = this->find(key); it != this->end()){
                    return cv_pred(it->second);
                }

                return false;
            });

            if(waited){
                return interrupt ? ENWaiterStatus::En_interrupt : ENWaiterStatus::En_success;
            }

            return ENWaiterStatus::En_timeout;
        }
    };

    template<class K, class V, class Mtx = NonMutex>
    class map_pointer : public MapEx<K, std::shared_ptr<V>, Mtx>
    {
        using MapEx<K, std::shared_ptr<V>, Mtx>::MapEx;
    public:
        template<class Creator>
        std::shared_ptr<V> get_or_insert(const K & key, Creator && creator)
        {
            auto obj = get_value(key);
            if(!obj){
                obj = creator();
                this->insert(key, obj);
            }

            return obj;
        }
    };

    template<class K, class V>
    class vector_map : public MapEx<K, std::vector<V>>
    {
        using MapEx<K, std::vector<V>>::MapEx;
        using MapEx<K, std::vector<V>>::end;
        using MapEx<K, std::vector<V>>::find;
    public:
        void push_back(const K & key, const V & val)
        {
            auto & self = *this;//忘记加引用符号，闹出乌龙
            auto it = this->find(key);
            auto end = self.end();
            if(it == end){
                this->try_emplace(key, std::vector<V>{});
            }

            self[key].emplace_back(val);
        }
    };



    }//namespace map
}//namespace Helper

#endif // STD_MAP_EX_HPP
