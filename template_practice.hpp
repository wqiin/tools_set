#ifndef TEMPLATE_PRACTICE_HPP
#define TEMPLATE_PRACTICE_HPP

#include <type_traits>
#include <vector>
#include <iostream>



//1 写一个 trait：has_size<T> // 判断 T 是否有 .size() 成员函数
struct AA{
};

struct BB{
    int size(){return 0;}
    //int size(int){return 0;};
};
struct CC{
    int size(int){return 0;};
};

struct DD{
    int size(int, double){return 0;};
};


template<class T, typename = void>
inline constexpr bool has_size = false;


//这里需要使用std::voie_t实现，将第二个参数变为void以匹配主模板
template<class T>
inline constexpr bool has_size<T, std::void_t<decltype(std::declval<T>().size())>> = true;

void has_size_usage()
{
    constexpr bool has_size_1 = has_size<BB>;//true
    constexpr bool has_size_3 = has_size<std::vector<int>>;//true,需要传入类型
    constexpr bool has_size_2 = has_size<AA>;
}


//1.2，如果size有参数怎么办呢
template<class T, typename = void>
inline constexpr bool has_size_with_ret = false;


//这里需要使用std::voie_t实现，将第二个参数变为void以匹配主模板
template<class T>
inline constexpr bool has_size_with_ret<T, std::void_t<decltype(std::declval<T>().size(std::declval<int>()))>> = true;
void has_size_with_retusage()
{
    constexpr bool has_size_1 = has_size_with_ret<BB>;//false
    constexpr bool has_size_3 = has_size_with_ret<std::vector<int>>;//false, vector需要传入类型，使其变为一个类型
    constexpr bool has_size_2 = has_size_with_ret<CC>;//true,存在一个size函数，且参数为int类型
}

//1.3 如果size有多个参数怎么办
//declval只返回类型，而不会调用，在其源码中已经限制了
template<class T, class ...Args>
using size_expr = decltype(std::declval<T>().size(std::declval<Args>()...));

template<class T, class, class...Args>
inline constexpr bool has_size_template = false;

template<class T, class...Args>
inline constexpr bool has_size_template<T, std::void_t<size_expr<T, Args...>>, Args...> = true;



//2. 检测嵌套类型 has_value_type<T> // 判断 T::value_type 是否存在
//这里是常量 实现
template<class T, typename  = void>
inline constexpr bool has_value_type = false;

template<class T>
inline constexpr bool has_value_type<T, std::void_t<typename T::value>> = true;


//这里是类型实现
template<class T, typename  = void>
struct has_value_t : std::false_type{};

template<class T>
struct has_value_t<T, std::void_t<typename T::value>> : std::true_type{};




struct FF{
    using value = int;
};

//2.1 如果value是private成员呢？
struct GG{
private:
    using value = float;

    //使用友元函数
    template<class, class>
    friend struct has_value_t;
};

void has_value_type_usage()
{
    constexpr bool has_value = has_value_type<FF>;//true
    constexpr bool has_value_1 = has_value_type<AA>;//false

    constexpr bool has_value_2 = has_value_type<GG>;//false, value为private，不能被外部访问

    constexpr bool has_value_3 = has_value_t<GG>::value;//true, value为private，但是has_value_t是GG的友元类，因此GG:value可以被外部访问
}


//3. std::void_t为什么它可以用于 SFINAE？ 为什么“所有参数都变成 void”还能起作用？
//std::void_t源码
template<typename...>
using void_t = void;

/*
std::void_t<typename T::type>
编译器做的步骤是：
先看 typename T::type 合不合法
如果合法 → 才继续
然后把整个 void_t<...> 变成 void,特化成功
如果不合法，则替换失败，当前特化被丢弃，触发SFINAE

void_t 只是一个“触发检查的载体”

SFINAE 起作用是因为“模板参数替换时会检查表达式是否合法”
👉 void_t 的作用是：
    触发这个检查
    然后把所有成功情况统一成 void，方便匹配

总结:
std::void_t 能用于 SFINAE，不是因为它是 void，而是因为它“在变成 void 之前，强制实例化并检查了模板参数里的表达式”。
👉 变成 void 的目的，是把所有成功情况统一成一个类型，从而让模板特化可以匹配

    ***   在模板匹配中，所有模板参与匹配，但是会选择最特化的那个版本，即，特化模板的匹配优先级较高****

*/



//4. 实现一个 enable_if
template<bool cond, typename T = void>
struct enable_if_my{
};

template<typename T>
struct enable_if_my<true, T>{
    using type = T;
};


void enable_if_my_usage()
{
    //std::enable_if<false>;
}


//5. 写一个通用 add,要求：返回正确类型（考虑 int + double), 不要写死返回类型
template<class T, class U>
std::common_type_t<T, U> general_add(T t, U u)
{
    using Result_t = std::common_type_t<T, U>;

    Result_t sum = t + u;
    return sum;
}


template<typename T, typename U>
auto add(T a, U b)
{
    return a + b;
}

//用 std::common_type（另一种思路），common_type ≠ decltype(a + b)，两者不一定完全等价，有些类型（比如自定义运算符）可能不一致
template<typename T, typename U>
auto add(T a, U b) -> std::common_type_t<T, U>
{
    return a + b;
}

template<class T, class Y>
auto add(T t, Y u)->decltype( u + t){//显示指明返回类型，也可以省略
    return t + u;//auto + return 会自动推导返回类型,推导规则就是 decltype(a + b)
}

//完美转发版本（更泛型），优点：支持左值 / 右值，避免不必要拷贝
template<typename T, typename U>
auto add(T&& a, U&& b)
    -> decltype(std::forward<T>(a) + std::forward<U>(b))
{
    return std::forward<T>(a) + std::forward<U>(b);
}

//C++20 进阶（加约束）
template<typename T, typename U>
requires requires(T a, U b) {
    a + b;
}
auto add(T a, U b)
{
    return a + b;
}


//6. 判断函数是否可调用 is_callable<F, Args...>
template<class Func_t, class...Args>
inline constexpr bool is_callable(Func_t && func, Args&& ... args)
{
    return std::is_invocable_v<Func_t &&, Args&&...>;
}


void is_callable_usage()
{
    constexpr bool is_callable_ = is_callable(enable_if_my_usage);//true

    int abc = 23;
    constexpr bool is_callable_1 = is_callable(abc);//false

    auto local_lambda = [](int , float)
    {

    };

    constexpr bool is_callable_2 = is_callable(local_lambda, "hello world", 23);//false
}

//7. 实现一个safe_calling函数，要求：如果 f 能调用 → 调用；否则 → 什么都不做 / 或返回默认值


template<class Func_t, class...Args>
using FunRet_t = std::invoke_result_t<Func_t, Args...>;

//->FunRet_t<Func_t, Args...>

template<class Func_t, class...Args>
auto safe_calling(Func_t && fun, Args&& ...args)
{
    if constexpr(std::is_invocable_v<Func_t&&, Args&&...>){
        return std::invoke(std::forward<Func_t>(fun), std::forward<Args>(args)...);
    }else{
        return nullptr;
    }
}

//concept写法
template<class Func_t, class...Args>
    requires std::is_invocable_v<Func_t, Args...>
decltype(auto) safe_calling(Func_t && func, Args&&... args)
{
    return std::invoke(std::forward<Func_t>(func), std::forward<Args>(args)...);
}


//8. 判断一个类型是否可以支持memcpy
template<class T>
inline constexpr bool is_memcpy = std::is_trivially_copyable_v<T>;



template<class T>
struct Adder{
    T operator()(const T & a, const T & b){
        return a + b;
    }

    //operator() 必须是成员函数
    void operator()(){
        std::cout << "Default reload 'operator' functionality.";
    }


    //explicit如果不加，可能会隐式转换到int或者其他整数类型，
    //&符号 - 限定仅左值对象调用
    [[nodiscard("Ret value being IGNORED.")]]inline constexpr explicit operator bool() const & noexcept {
        return true;
    }

    //&&符号 - 只限定有值对象调用
    [[nodiscard("Ret value being IGNORED.")]]inline constexpr explicit operator bool() const && noexcept {
        return true;
    }
};


void adder_usage()
{
    Adder<int> adder;
    adder(2, 3);

    adder.operator()();;//手动调用重载运算符
    bool b = adder.operator bool();//手动调用bool重载运算方法


}


#include <atomic>
template<typename T>
class LockFreeQueue
{
private:
    struct Node
    {
        T value;
        std::atomic<Node*> next;

        Node(T v)
            : value(std::move(v)),
            next(nullptr)
        {
        }
    };

public:
    LockFreeQueue()
    {
        Node* dummy = new Node(T{});

        head.store(dummy);
        tail.store(dummy);
    }

    ~LockFreeQueue()
    {
    }

    void push(T value)
    {
        Node* new_node = new Node(std::move(value));

        while (true)
        {
            Node* old_tail = tail.load();

            Node* null_node = nullptr;

            if (old_tail->next.compare_exchange_weak(null_node, new_node))//空队列，使其指向新生成的节点
            {
                tail.compare_exchange_weak(old_tail, new_node);
                return;
            }
            else
            {
                tail.compare_exchange_weak(old_tail, old_tail->next.load());
            }
        }
    }

    bool pop(T& result)
    {
        while (true)
        {
            Node* old_head = head.load();
            Node* next = old_head->next.load();

            if (!next){//空队列
                return false;
            }

            if (head.compare_exchange_weak(old_head, next))//如果head被其他线程修改，则重新循环，再取
            {
                result = std::move(next->value);

                delete old_head;
                return true;
            }
        }
    }

private:
    std::atomic<Node*> head;
    std::atomic<Node*> tail;
};









#endif // TEMPLATE_PRACTICE_HPP
