#ifndef TEMPLATE_PRACTICE_HP  P
#define TEMPLATE_PRACTICE_HPP

#include <type_traits>
#include <vector>
#include <iostream>
#include <memory>
#include <thread>
#include <functional>



template<typename T, typename = void>
struct has_clear_int : std::false_type {};
template<typename T>
struct has_clear_int<T, std::void_t<decltype(std::declval<T>().clear(std::declval<int>()))>> : std::true_type {};

//检测一个类型是否支持 + 运算符（两个const T&相加）：
template<typename T, typename = void>
struct is_addable :std::false_type {};

template<typename T>
struct is_addable<T, std::void_t<decltype(std::declval<const T&>() + std::declval<const T&>())>> : std::true_type {};

template<typename T, typename = void>
struct is_range : std::false_type {};

//void_t 是个变参模板，它会把所有 decltype 都求值一遍（实际是检查合法性）。只要有一个非法，整个替换就失败，偏特化丢弃。
template<typename T>
struct is_range<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),

    // 可以加更多要求，比如 begin() 返回的迭代器支持 != 比较
    decltype(std::declval<T>().begin() != std::declval<T>().end())
    >> : std::true_type {};



template<class T>
struct printerImpl{
    static void print(const T & t){}

    static constexpr size_t buffer_size = 32;
    static constexpr size_t buffer_align = alignof(std::max_align_t);

};



template<>
struct printerImpl<int>{
    //特化版本的函数，所以函数签名可以完全不同
    static void print(int t){
    }
};


template<class T>
class printer{
public:
    void print(const T & t){
        printerImpl<T>::print(t);
    }
};


//enable_shared_from_this 允许从 this 安全地获得一个 shared_ptr。
class Worker : public std::enable_shared_from_this<Worker> {
    public:
        void startAsync() {
            // 获取 shared_ptr<Worker>
            std::weak_ptr<Worker> weak_this = shared_from_this();
            std::thread t([weak_this]() {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                    if (auto sp = weak_this.lock()) {// 提升为 shared_ptr
                        sp->doWork();
                    }
                }
            );

            t.detach();
        }

        ~Worker() {
            std::cout << "Worker destroyed" << std::endl;
        }

        void doWork(){
            std::cout << "Worker working..." << std::endl;
        }
    };

void shared_from_this_usage() {
    auto w = std::make_shared<Worker>();
    w->startAsync();
    std::cout << "Waiting for the thread to execute." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::function<void()> fun;
}

template<typename T>
    auto get_size(const T& t)
    {
        if constexpr (requires { t.size(); })// C++20 的 requires
        {
            return t.size();
    }else if constexpr (std::is_array_v<T>)
    {
        return sizeof(T) / sizeof(std::decay_t<decltype(t[0])>);
    }else
    {
        return 0;
    }
}


//C++14 允许普通函数模板直接写 auto 返回类型，编译器会自动推导，而且推导过程仍然遵循 SFINAE——如果推导失败，这个重载就会被丢弃。
template<typename C>
auto front(C& c)
{
    return c.front();// 如果 C 没有 front()，推导失败 -> SFINAE 丢弃此模板
}


// 检测 T 是否有 foo() 成员函数
template<typename T>
struct has_foo{
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().foo(), std::true_type{});

    template<typename U>
    static auto test(...)->std::false_type;

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};


//实现方法2
template<typename, typename = void>
struct has_foo_2 : std::false_type
{
};

template<typename T>
struct has_foo_2<T, std::void_t<decltype(std::declval<T>().foo())>> : std::true_type
{
};

//检测静态成员函数 foo是否存在
template<typename, typename = void>
struct has_static_member : std::false_type
{
};

template<typename T>
struct has_static_member<T, std::void_t<decltype(T::foo())>> : std::false_type
{
};


//编译期判断一个类型是不是 std::vector
template<typename T>
struct is_vector : std::false_type {};

// 偏特化：匹配任何 std::vector<U, Alloc>
template<typename U, typename Alloc>
struct is_vector<std::vector<U, Alloc>> : std::true_type {};

static_assert(is_vector<std::vector<int>>::value);// true
static_assert(!is_vector<int>::value);// false




//目标：typelist<int, double, char> 转成 typelist<int*, double*, char*>。
template<class ...Args>
struct typelist{
};

template<typename List, template<typename> class MetaFunc>
struct map;

        // 递归边界
template<template<typename> class MetaFunc>
struct map<typelist<>, MetaFunc>
{
    using type = typelist<>;
};

// 递归步骤
template<typename Head, typename... Tail, template<typename> class MetaFunc>
struct map<typelist<Head, Tail...>, MetaFunc>
{
    using type = typelist<
    typename MetaFunc<Head>::type, // 变换 Head
        typename map<typelist<Tail...>, MetaFunc>::type// 递归处理 Tail
        >;
};

// 元函数：加指针
template<typename T>
struct add_pointer
{
    using type = T*;
};

using Original = typelist<int, double, char>;
using Result = map<Original, add_pointer>::type;    // typelist<int*, double*, char*>


//这是数学里的：全序关系（total order），意思是：任意两个元素都能比较
//C++20，以判断类型是否支持完整的大小关系比较，即同时满足==、!=、 <、<=、>、>=这些都合法。并且：逻辑一致
template<class T>
constexpr bool is_total_ordered = std::totally_ordered<T>;


//C++20的concept， 意思是：两个对象能判断：“是否相等”，即==、！=运算符合法
template<class T>
constexpr bool is_compared = std::equality_comparable<T>;



//编译期间和运行期间的不同实现
constexpr int func_mluti(int x)
{
    if (std::is_constant_evaluated())//编译期间执行的分支 -C++20 ss
    {
        return x * 2;
    }
    else//运行时执行的分支
    {
        return x * 3;
    }
}


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


//*****模板有特殊规则：编译器允许它们在多个翻译单元重复出现，因此多个cpp文件中include相同的头文件，也不错编译错误
//函数模板，更多是为了生存运行期的代码，而不是在编译期间调用

//constexpr 函数 ≠ 一定编译期执行

//6. 判断函数是否可调用 is_callable<F, Args...>
template<class Func_t, class...Args>
inline constexpr bool is_callable(Func_t && func, Args&& ... args)
{
    return std::is_invocable_v<Func_t &&, Args&&...>;//加上&&，以保留Func_t原有的引用属性；
}

//判断函数是否可调用，不需要实现为函数 - 元编程是在编译器得到，而函数是在运行时得到结果，
template<class Func_t, class...Args>
constexpr bool fun_is_callable = std::is_invocable_v<Func_t &&, Args&&...>;






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
