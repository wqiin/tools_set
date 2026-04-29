#ifndef TEMPLATE_DEMO_HPP
#define TEMPLATE_DEMO_HPP

#include <algorithm>
#include <iostream>
#include <type_traits>
#include <memory>
#include <functional>
#include <future>

#include <source_location>//C++20

#include "allocator.hpp"


//使用完美转发，调用传入的可调用对象z
template<class L, class R, class T>
auto hello_lambda(L && left, R && right, std::function<T(L, R)> && func)
{
    return std::invoke(func, std::forward<L>(left), std::forward<R>(right));
}

//讲传入的可调用对象和参数，打包到异步执行的std::packaged_task对象中
template<class Func_t, class ...Args>
decltype(auto) hello_world(Func_t && func, Args&& ...args)
{
    // static_assert(std::is_invocable_v<Func_t&&, Args&&...>, "Func is not invocable with given arguments.");//调用发生在转发后的类型
    // return std::invoke(std::forward<Func_t>(func), std::forward<Args>(args)...);

    using Result_t = std::invoke_result_t<Func_t , Args...>;
    using Result__t = typename std::invoke_result<Func_t , Args...>::type;

    //需要使用完美转发到std::bind的对象中
    auto p = std::make_shared<std::packaged_task<Result_t()>>(std::bind(std::forward<Func_t>(func), std::forward<Args>(args)...));

    // //将calling放入到容器里面，依次调用即可
    // auto calling = [p]{
    //     *p();
    // };

    // return p->get_future();
}


//使用C++20的std::source_location的类，定位文件，函数和行号
template<class T>
T add(T a, T b){
    std::source_location loc = std::source_location::current();//
    std::cout << "file name: " << loc.file_name() << "  line id: " << loc.line() << "  func name: " << loc.function_name() << std::endl;

    return a + b;
}


//标签分发
//std::true_type;//就是一个编译期的值，表示为真，
template<class T>
void impl(T , std::true_type)
{
    std::cout << "integral\n";
}

template<class T>
void impl(T , std::false_type)
{
    std::cout << "non integral\n";
}

template<class T>
void tag_dispatch(T x){
    impl(x, std::is_integral<T>{});//标签分发

    //std::is_integral<T>{}返回的是bool的类型对象
    //std::is_integral_v<T>；//,返回的是一个编译期的bool值
}


//这里的Container_t是一个类型，而不是模板，指示这个类型依赖于第一个类型，仅此而已，如果需要传模板作为参数
template<class T, class Container_t = std::deque<T>>
class customed_queue{
};

//template<class, class...>表示模板，至少需要一个参数，且支持任意数量的模板参数
template<class T, template<class, class...> class Container_t = std::deque>
class queue_template{
};


void customed_inpassed_type()
{
    customed_queue<int, std::deque<int>> hello;//这里传入为实例化的类型
    queue_template<int, std::deque> world;//这里传入的一个模板类型
}


//前置声明
template<class T>
void func();

template<class T>
void show(T & t);


template<class T>
void show(T & t)
{
    std::cout << t.t << std::endl;
}


//类模板
template<class T>
class A{
private:
    T t;
    static inline int count = 0;//不同类型的T不共享该变量，仅在相同类型的共用

public:
    A() = default;
    A(const A & a);
    ~A() = default;

    //约束模板友元函数
    friend void func<T>();
    friend void show<>(A<T> & a);

    //非约束模板友元函数,不同有类型T；在类内声明，
    template<class V, class U>
    friend void show(V & t, U & u);
};

template<class T>
void func()
{
    std::cout << A<T>::count << std::endl;//因为是友元函数，可以访问private变量
}

//在类外定义
template<class V, class U>
void shwo(V & v, U & u)
{
    std::cout << v.t << u.t << std::endl;
}

//类外实现成员函数
template<class T>
A<T>::A(const A & a)
{
    this->t = a.t;
}



//非类型模板参数类
//MAX_SIZE作为一个非类型参数，只支持char,int,bool类型，在C++20后支持一些严格要求的自定义结构体类型
template<class T, std::uint32_t MAX_SIZE>
class array{
private:
    T data[MAX_SIZE];

public:
    array();
    void init() noexcept;

    T& at(int nIndex) noexcept;
    T& operator[](int nIndex) noexcept;
    std::uint32_t size() const noexcept;
};

template<class T, std::uint32_t MAX_SIZE>
array<T, MAX_SIZE>::array()
{
    init();
}

template<class T, std::uint32_t MAX_SIZE>
T& array<T, MAX_SIZE>::at(int nIndex) noexcept
{
    if(nIndex >= MAX_SIZE){
        throw std::runtime_error("Out of Index.");
    }

    return data[nIndex];
}

template<class T, std::uint32_t MAX_SIZE>
T& array<T, MAX_SIZE>::operator[](int nIndex) noexcept{
    return data[nIndex];
}


template<class T, std::uint32_t MAX_SIZE>
std::uint32_t array<T, MAX_SIZE>::size()const noexcept
{
    return MAX_SIZE;
}

template<class T, std::uint32_t MAX_SIZE>
void array<T, MAX_SIZE>::init() noexcept
{
    std::memset(data, 0x00, sizeof(T) * MAX_SIZE);
}



//模板特化
template<class T>
bool is_equal(const T & t1, const T & t2)
{
    if constexpr(std::is_floating_point_v<T>){
        const T epsilon = std::numeric_limits<T>::epsilon();
        // return std::fabs(t1 - t2) <= epsilon * std::max(T(1.0), std::max(std::fabs(t1), std::fabs(t2)));

        const T abs_epsilon = static_cast<T>(1e-6);   // 绝对误差，处理接近0
        const T rel_epsilon = static_cast<T>(1e-6);   // 相对误差， 处理较大的数

        T diff = std::fabs(t1 - t2);
        if (diff <= abs_epsilon) {
            return true;
        }

        return diff <= rel_epsilon * std::max(std::fabs(t1), std::fabs(t2));
    }else{
        return t1 == t2;
    }
}

//特化的形式传参数，需要与非特化版本一致，范性版本传入的是const T &, 特化版本也需要传入这样的版本
template<>
bool is_equal<char *>(char* const & x, char* const & y){
    return strcmp(x, y);
}

template<>
bool is_equal<double>(const double & t1, const double & t2)
{
    const double epsilon = std::numeric_limits<double>::epsilon();
    // return std::fabs(t1 - t2) <= epsilon * std::max(T(1.0), std::max(std::fabs(t1), std::fabs(t2)));

    const double abs_epsilon = static_cast<double>(1e-6);   // 绝对误差，处理接近0
    const double rel_epsilon = static_cast<double>(1e-6);   // 相对误差， 处理较大的数

    double diff = std::fabs(t1 - t2);
    if (diff <= abs_epsilon) {
        return true;
    }

    return diff <= rel_epsilon * std::max(std::fabs(t1), std::fabs(t2));
}

template<>
bool is_equal<float>(const float & a, const float & b){
    const double epsilon = 1e-9;
    return std::fabs(a - b) <= epsilon * std::max(1.0f, std::max(std::fabs(a), std::fabs(b)));
}


void hello()
{
    //std::erase_if();

    //std::is_same<int, int>::value;//判断两个类型是否相同

    std::shared_ptr<int> p(new int(23), [](int *p) { delete p;});//构造函数中，传入自定义的删除器

    using multi_d_arr = int[2][3];
    multi_d_arr arr = {0};//二维数组



    //--------在编译器求值得到类型
    //std::extent_v 返回多维数组第一个维度的长度
    constexpr size_t first_demo = std::extent_v<multi_d_arr>;

    //std::remove_extent_t返回多维数组降低一维后的类型
    /*
        返回数组降低一个维度后的数据类型。不改变数据类型的限制属性(const, volatile, const volatile)
         一维数组降低到0维度；
         二维数组降低到一维数组；
         三维数组降低到二维数组；
    */
    using degrade_arr = std::remove_extent_t<multi_d_arr>;

    //std::remove_all_extents_t 移除所有维度后的数组类型
    using original_type = std::remove_all_extents_t<multi_d_arr>;

    //std::rank_v得到多维数组的维度数
    constexpr size_t dems = std::rank_v<multi_d_arr>;

    constexpr size_t dems1 = std::rank_v<int>;

    //std::negate 是 C++ 标准库中的一个函数对象（也称为函数器或仿函数），它位于 <functional> 头文件中。std::negate 的作用是对给定的数值取反，即返回其相反数。它是一个模板类，可以用于不同的数值类型，如 int、float、double 等。
    // 创建 std::negate 对象
    std::negate<int> neg;
    int num = 23;
    int result = neg(23);

    //判断整数类型是否为有符号
    constexpr bool is_signed = std::is_signed_v<int>;
    constexpr bool is_unsigned = std::is_unsigned_v<int>;
}



//std::enable_if在concept之前，用于控制传入指定类型的用法，用到了SFINAE
template <typename T,
         typename std::enable_if<std::is_integral<T>::value && !std::is_same<T, bool>::value, int>::type = 0>
T safe_divide(T a, T b)
{
    static_assert(!std::is_same<T, bool>::value,"bool类型不支持除法");
    return b != 0 ? a / b : 0;
}
/*这里的 = 0是在干啥，
自我理解：就是这里使用了std::enable_if这个方法来控制传入T类型，因而新增了一个参数类型，但是在调用方又需要无感地处理这个类型参数，因而设置为0，避免在使用时，手动传入这个参数
= 0 的作用是：给这个“模板参数”一个默认值，从而让调用时不需要显式传它。
🧠 一句话核心
    = 0 让这个 SFINAE 参数“隐身”，调用函数时你不用管它
🔍 一、先还原一下完整含义
    template <typename T,typename std::enable_if<条件, int>::type = 0>
    等价理解为：template <typename T,int = 0>   // 只有当条件成立时，这一行才存在,即这个类型有一个默认值

🧠 二、如果没有 = 0 会怎样？
    template <typename T,typename std::enable_if<条件, int>::type >

👉 那你调用时必须写：
            safe_divide<int, 0>(a, b); // ❌ 很蠢

👉 因为第二个模板参数是“必填的”

✅ 三、有了 = 0 之后
        safe_divide(10, 2);
👉 编译器自动补：
        safe_divide<int, 0>(10, 2);
🧠 四、为什么是 int + = 0？
    看这一段：
            std::enable_if<条件, int>::type
👉 含义：
        如果条件成立 → type = int
        如果条件不成立 → 没有 type（SFINAE 生效）
        所以整体变成：
    int = 0
*/

/*
 原理其实不复杂。enable_if利用的是SFINAE（Substitution Failure Is Not An Error）
规则：模板参数替换失败时编译器不报错，默默跳过这个重载去找下一个能匹配的。
enable_if就是制造"替换失败"的工具——条件不满足时enable_if<false>里没有type成员，替换失败触发SFINAE，这个重载就被踢出候选集了。
即，模板匹配不报错，而是匹配下一个模板，如果没有则报错
*/



//std::enable_if用法
// 写法1：塞在返回值里
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T value){
    return value * 2;
}

//这里将上述的std::enable_if的改写
template<class T>
std::enable_if_t<std::is_integral_v<T>, T> get_default_value()
{
    return T{};
}

// 写法2：塞在模板参数里
template<typename T,
         typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
T process(T value){
    return value * 2;
}

// 写法3：塞在函数参数里
template<typename T>
T process(T value,
          typename std::enable_if<std::is_integral<T>::value>::type* = nullptr){
    return value * 2;
}

// C++20 concepts 写法，传入的模板参数需要为整数
template<std::integral T>
T safe_divide(T a, T b)
{
    return b != 0 ? a / b : 0;
}

//自定义的concept,用于限制传入类型
template<class T>
concept Numberic = std::integral<T> || std::floating_point<T>;

template<Numberic T>
T safe_divide(T a, T b)
{
    return b != 0 ? a / b : 0;
}

//需要为有符号的数字类型 - concept多一系列条件的约束，条件不影响在模板中的类型
template<typename T>
concept SignedNumeric = Numberic<T> && std::is_signed_v<T>;

template <SignedNumeric T>
inline T absolute(T val)
{
    return val < 0 ? -val : val;
}


void template_usage()
{
    std::int32_t uint = 32;
    absolute(uint);
}


//#include <immintrin.h>

//带主副模板的特化 - 主要模版
template<class T, bool = std::is_arithmetic_v<T>>
class default_val{
public:
    default_val()
    {
        std::cout << "Not Special Template calling\n";
    }
};

//特化版本
template<class T>
class default_val<T, true>{
public:
    default_val()
    {
        std::cout << "Number Template calling\n";
    }
};

template<>
class default_val<bool, true>{
public:
    default_val()
    {
        std::cout << "Bool Template calling\n";
    }
};


template<>
class default_val<float, true>{
public:
    default_val()
    {
        std::cout << "float Template calling\n";
    }
};

template<>
class default_val<double, true>{
public:
    default_val();
    // {
    //     std::cout << "double Template calling\n";
    // }
};

// template<> //构造函数不需要再加上这个特化声明标签了，因为本身已经是特化的构造函数
default_val<double, true>::default_val()
{
    std::cout << "double Template calling\n";
}



//C++14支持的特化方法
struct AA; struct BB;

template<class T>
constexpr size_t fun = 0;

//特化
template<>
constexpr size_t fun<AA> = 1;

template<>
constexpr size_t fun<BB> = 2;

void cpp14_template_usage()
{
    size_t var = fun<AA>;
}


//这里的std::enable_if_t<is>相当于std::enable_if_t<is, void>，即std::enable_if_t<is>*，当is为真时，类型为void*
//这里为C++的SFINAE，匹配失败并非错误
template<bool is, class T, std::enable_if_t<is>* = nullptr>
auto hello_world(T && t)
{
}

template<bool is, class T, typename std::enable_if_t<!is>* = nullptr>
auto hello_world(T && t)
{
}

//C++14及以后，函数声明中可以不用显示指明其返回，编译器可以自动推导return返回的类型，但需要return语句返回的类型都相同

//C++17的constexpr在编译器求值，已确定编译路径
template<bool check>
auto func()
{
    if constexpr(check){
        return (int)0;
    }else{
        return (double)0;
    }
}


//在编译期求整数二进制中1的个数
template<size_t input>
constexpr size_t ones_count = (input % 2) + ones_count<(input / 2)>;

//特化版本,以用来充当结束循环的分支
template<>
constexpr size_t ones_count<0> = 0;

//因为是在编译器求值，所以不用在函数中被调用
constexpr size_t count = ones_count<23>;


//使用编译期，循环处理数据，不用于确定参数个数
template<size_t... inputs>
constexpr size_t Accumulate = 0;

template<size_t cur_input, size_t...inputs>
constexpr size_t Accumulate<cur_input, inputs...> = cur_input + Accumulate<inputs...>;

constexpr size_t sum_accumulate = Accumulate<1, 2, 3, 4, 5>;


//使用C++17的折叠表达式实现上述功能
template<size_t ... vals>
constexpr auto fun_accumulate()
{
    return (0 + ... + vals);
}

constexpr size_t result_fun_accumulate = fun_accumulate<1, 2, 3, 4, 5>();


//函数模版不能被声明为虚函数，静态函数也无法被声明为虚函数
//奇异的递归模版 - CRTP

template<class T>
struct Base{
    template<class U>
    void func(const U & u){
        T * pT = static_cast<T*>(this);
        pT->impl(u);
    }

    static void func_static()
    {
        T::impl_static();
    }
};

struct Derive : public Base<Derive>{
    template<class T>
    void impl(const T & t){
        std::cout << t << std::endl;
    }

    static void impl_static()
    {
        std::cout << "Implate from static function in derived class" << std::endl;
    }
};

void crtp_usage()
{
    Derive d;
    d.func("Hello World");

    Derive::func_static();//调用基类的犯法
}


//practice - 练习
template<class T, size_t SIZE_OF>
constexpr bool judge_size = (sizeof(T) == SIZE_OF);

constexpr bool int_size = judge_size<int, 5>;

template<class T>
constexpr size_t get_size = sizeof(T);

constexpr size_t double_szie = get_size<double>;


//使用SFINAE构造一个元函数，输入一个类型T，如果T存在子类型type返回true，否则返回false

//为什么要写 typename = std::void_t<>
//1️⃣ 提供一个“可替换的位置”
//2️⃣ 让偏特化有机会“匹配 or 失败”
//3️⃣ 利用 SFINAE 选择不同版本
//typename = std::void_t<>只是一个占位符，类型为void
template<class, typename = std::void_t<>>
struct has_type : std::false_type{};

//std::void_t不管传入什么类型，都返回void，前提是传入的类型和法，否则就匹配上面的默认has_type模版
// ---- 这种办法更好，因为得到的是std::true_type这种将常量类型化的类型
template<class T>
struct has_type<T, std::void_t<typename T::type>> : std::true_type{};

struct AA{
    using type = int;
};

struct BB{
};

constexpr bool a_has_type = has_type<AA>::value;


//另一种实现方法
//SFINAE：替换失败 ≠ 编译错误，而是“这个候选版本被丢弃”
template<class T, typename = void>
inline constexpr bool exists_type = false;

//对第二个模板参数的特化版本
template<class T>
constexpr bool exists_type<T, std::void_t<typename T::type>> = true;

//在编译器尝试实例化模板时，会优先尝试特化的版本

constexpr bool a_has_type_2 = exists_type<AA>;
constexpr bool b_has_type_2 = exists_type<BB>;


// 成功版本：T::type 存在
//decltype(typename T::type{}, std::true_type{})， typename T::type{}和法，则返回std::true_type{}类型


//使用循环代码写法，写一个元函数，输入一个类型数组，输出一个无符号整型数组，输出数组的每一个元素表示输入数组对应类型变量的大小
template<class... Ts>
constexpr std::array<size_t, sizeof...(Ts)> type_size_array()
{
    return {sizeof(Ts)...};
}

auto arr = type_size_array<double, int, float>();


//SFINAE限定传入的参数满足其中的条件，否则跳过这个模板的匹配
template<class T>
std::enable_if_t<std::is_integral_v<T>, T> get_integral_default(){
    return T{};
}

void enable_usage()
{
    //get_integral_default<float>();//上述enalbe_if不满足条件，则无法匹配到get_integral_default函数，直接编译报错
    get_integral_default<int>();//满足条件
}

//参数控制 - 不满足std::enable_if_t的条件是，这个模版函数是直接不存在，
//为什么要写 = 0？给这个“隐藏模板参数”一个默认值；这样用户调用时：func(10); // 不需要写第二个参数
template<typename T, typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
void func(T x)
{
}

//（别名简化） - 满足条件时，enable_if_t类型为默认的void, 否则类型不存在

struct enable_type_false{
    static inline constexpr bool value = false;
};

struct enable_type_true{
    static inline constexpr bool value = true;
};

template<typename T>
using enable_if_t = typename std::enable_if_t<T::value, int>;

//enable_if_t<enable_type_false> a = 1;//enable_type_false中的value为假，编译报错
enable_if_t<enable_type_true> aa = 1;//enable_type_false中的true为真，得到默认类型


//std::enable_if_t与std::conditional_t的区别
//👉 enable_if 是“让代码存不存在”
//👉 conditional 是“在两个类型里选一个”


//template<typename T>
//using customed_type = std::enable_if_t<false, int>;//当条件为假时，这段代码不会存在

//用条件控制，在类型中二选一
using T = std::conditional_t<true, int, double>; // int


template<class T>
using void_type = std::is_void<T>::value;

constexpr bool is_void = std::is_void_v<int>;//判断类型是否为void

constexpr bool is_compound = std::is_compound_v<void>;//除了整数、浮点数和void，其他都是复合类型
constexpr bool is_fundamental = std::is_fundamental_v<void>;//整数、浮点数和void返回true,其他都是返回false

template<class T>//scalar = “能当一个单独数值用的类型”（最基础可操作单元）
constexpr bool is_scalar = std::is_scalar_v<T>;


//object type = 除函数、引用、void 之外的所有类型
template<class T>
constexpr bool is_object_t = std::is_object_v<T>;

//C++ 中 “trivial” 的核心含义：👉 可以用 C 语言那种“纯内存操作”方式安全处理
//平凡默认构造、平凡复制构造、平凡赋值操作符 - 什么都不做（编译器生成）
//平凡默认构造函数 = 编译器生成、无副作用、等价于“什么都没做”的构造函数


std::add_const_t<int> a_cst = 23;
std::add_cv_t<int> int_cv = 23;
std::add_pointer_t<int> pInt = nullptr;



//非类型参数模板 - 把“编译期值 true或者false封装成一个“类型”，用以元编程
template<bool x>
struct bool_{
    inline static constexpr bool value = x;

    using type = bool_<x>;
    using value_type = bool;

    operator bool() const {return x;}
};
//std::bool_constant<true>;//同上述的bool_本质一样
//它的核心用途是： 在“类型系统中表达 true/false”

using false_ = bool_<false>;
using true_ = bool_<true>;


template<class T, T val>
struct integral_c{
    inline static constexpr T value = val;

    using type = integral_c<T, val>;
    using value_type = T;
    using next = integral_c<T, val + 1>;
    using priori = integral_c<T, val - 1>;

    operator T() const {return val;};
};


//没有concept之前，对传入的参数做限制
template<typename T, std::enable_if_t<std::is_integral_v<T>, T> = 0>
T add_original(T a, T b)
{
    return a + b;
}

//Concept其实是一个语法糖，它的本质可以认为是一个模板类型的bool变量。定义一个concept本质上是在定义一个bool类型的编译期的变量。使用一个concept本质上是利用SFINAE机制来约束模板类型。
//使用concept后的写法
template<class T>
concept integral = std::is_integral_v<T>;

//concept约束模板函数 方法1
template<integral T>
T add_with_concept(T a, T b)
{
    return a + b;
}

//concept约束模板函数 方法2
template<typename T>
requires integral<T>
void add_with_reqiures(T v)
{

}

//concept约束模板函数 方法3
template<typename T>
void add_with_reqiures_1(T v) requires integral<T>
{

}

//concept约束模板函数 方法4
void add_with_concept_2(integral auto v)
{
}

//concept约束模板函数 方法5
template<integral auto v>
void concept_in_template()
{
    return v * v;
}

template<integral T>
auto f = [](T a)
{
    return a * a;
};

void template_usage_in_hpp()
{
    //在局部lambda中使用模板
    auto f0 = []<class T, typename = std::enable_if_t<std::is_integral_v<T>, T>>(T v)
    {
        return v * v;
    };

    //在lambda中使用concept
    auto f = []<integral T>(T a)
    {
        return a * a;
    };

    auto f1 = []<typename T> requires integral<T>(T v)
    {
        return v * v;
    };

    auto f2 = []<typename T> (T v) requires integral<T>
    {
        return v * v;
    };

    auto f3 = [](integral auto v){
        return v * v;
    };

    auto f4 = []<integral auto v>()
    {
        return v * v;
    };

    constexpr int ret = f0(2);
}


/*concept的本质是一个模板的编译期的bool变量，
因此它可以使用C++的与或非三个操作符。当然，
理解上也就跟我们常见的bool变量一样啦。
例如，我们可以在定义concept的时候，使用其他concept或者表达式，进行逻辑操作。
*/

//concept的整数
template<typename T>
concept Integral_t = std::is_integral_v<T>;


//concept的符号整数
template<typename T>
concept SignedIntrgral_t = Integral_t<T> && std::is_signed_v<T>;


//concept的无符号整数
template<typename T>
concept UnsignedIntegral_t = Integral_t<T> && !std::is_signed_v<T>;


template<typename T>
requires Integral_t<T> && std::is_signed_v<T>
T add_with_requires_2(T a, T b)
{
    return a + b;
}


// requires用在concept的定义，它表达了类型T的参数f，必须符合大括号内的模式，也就是能被调用。
// 也就是它是一个函数或者一个重载了operator()的类型
//在concept中使用requires限定传入参数的约定
template<typename T>
concept Callable_t = requires(T f)
{
    f();
};



//concept组合使用
struct foo{
    int foo = 0;
};

struct bar{
    using value_t = int;
    value_t data = 0;
};

struct baz{
    using value_t = int;
    value_t data = 0;
};

//只能实例化baz类型的模板类
template<typename T, typename = std::enable_if_t<std::is_same_v<T, baz>>>
struct Special{
};

//std::enable_if的另一种写法
template<typename T, typename std::enable_if_t<std::is_same_v<T, baz>> = 0>
struct Special_t{
};

template<class T>
using Ref_t = T&;

//类型约束- 这些约束需要同时满足s
template<class T>
concept NewConcept_t = requires{
    typename T::value_t;  //A) T MUST has an inner member named 'value'
    typename Special<T>;    //B) T MUST have a valid class template specialization for class template 'Special'
    typename Ref_t<T>;      //C) T MUST be a valid class alias template substitution
};

template<NewConcept_t T>
void template_test_for_concept_3(T t)
{
}

void usage_for_concept_template_1()
{
    //template_test_for_concept_3(foo{});   //failed for requirment A
    //template_test_for_concept_3(bar{});   //failed for requirment B
    template_test_for_concept_3(baz{});     //OK
}



//复合约束- 用于约束表达式的返回值的类型，如下所示 //->后面需要是一个concept约束
template<class T>
concept Compound_bound = requires(T x)
{
    {*x} -> std::same_as<typename T::inner>;    // the type of the expression `*x` is convertible to `T::inner`
    {x + 1} ->std::same_as<int>;                // the expression `x + 1` satisfies `std::same_as<decltype((x + 1))>`
    {x * 1} -> std::same_as<T>;                 // the type of the expression `x * 1` is convertible to `T`
};


//嵌套约束 - requires内部还可以嵌套requires
template<class T>
concept DefaultConstructible = std::is_default_constructible_v<T>;

template<class T>
concept CopyConstructible = std::is_copy_constructible_v<T>;


template<class T>
concept Destructible = std::is_destructible_v<T>;

template<class T>
concept CopyAssignable = std::is_copy_assignable_v<T>;//判断类型 T 是否“可以进行拷贝赋值”

template<class T>
concept SemiRegular = DefaultConstructible<T> && CopyConstructible<T> && Destructible<T> && CopyAssignable<T>&&
requires(T obj, size_t n)
{
    requires std::same_as<T*, decltype(&obj)>;//&obj 的类型是不是 T*，只要&运算符不被重载
    {obj.~T()} noexcept;//能显式调用析构函数且是 noexcept
    requires std::same_as<T*, decltype(new T)>;//被奇怪地重载会不满足
    requires std::same_as<T*, decltype(new T[n])>;//被奇怪地重载会不满足
    {delete new T};// 检查：new 出来的对象可以被 delete
    {delete new T[n]};
};



//template<typename Vec>
//using Scalar  = std::decay_t<decltype(Vec()[0])>;//得到容器内部元素的类型，这里默认了传入的Vec有默认构造，如果不存在则会编译报错

template<typename Vec>
using Scalar  = std::decay_t<decltype(std::declval<Vec>()[0])>;//得到容器内部元素的类型，这样，即使没有默认的构造函数，也不会报错

template<typename Vec>
auto norm(const Vec& vec) -> Scalar<Vec> {
    Scalar<Vec> result = 0;//Scalar<Vec>为传入容器的存放的类型
    for (size_t i = 0; i < vec.size(); i++) {
        result += vec[i] * vec[i];
    }
    return std::sqrt(result);
}

struct Point2 {
    float x;
    float y;

    auto size() const -> int {
        return 2;
    }

    auto operator[](int i) const -> float {
        return i == 0 ? x : y;
    }
};

void usage_of_template_in_cpp() {
    std::vector<double> a = { 1,2,2 };
    std::cout << "norm a: " << norm(a) << std::endl;
    Point2 b = { 3,4 };
    std::cout << "norm b: " << norm(b) << std::endl;


    int aa = 23;
    float bb = 23.0;


    /*
    * decltype(expr1, expr2) 👉 的意思是：检查 expr1 是否合法，然后返回 expr2 的类型
    *
    * 为什么会用这种写法？
    * 1.推导“某个表达式的类型”，如
    * template<typename T, typename U>
      using result_t = decltype(std::declval<T>() + std::declval<U>());

    2.SFINAE 检查
    *template<typename T>
    auto test(int) -> decltype(std::declval<T>().foo(), std::true_type{});

    template<typename>//模板参数没有名字表示：👉 “我接受任意类型，但我不关心它”
    auto test(...) -> std::false_type;//这里的 ... 是：可变参数（C 风格 ellipsis）这种函数的匹配优先级是最低的（比普通参数、比模板参数都低，一定是配合另一个“更优先”的版本：
    */
    //(a, b) → 结果是 b, b 是左值 → double&
    using Decl_t = decltype((aa, bb));//这里类型为float &
}


//C++ 规定：🚫 函数模板不允许偏特化！
/*
template<class T, typename = void>
constexpr bool has_attri()
{
    return false;
}

template<class T>
constexpr bool has_attri<T, std::void_t<typename T::value>>()
{
    return false;
}
*/


//函数偏特化
template<class T, class U>
void func_partical_specialization(T, U)
{
}

/*
//函数偏特化 - 被禁止在C++中，如果允许的话，函数重载会在这三个中造成歧义，编译器不知道选择哪一个模板实例化
********* -函数的重载和函数的偏特化两者在重载决议的时候产生歧义 ***********
template<class T>
void func_partical_specialization<T, int>(T, int)
{
}
*/

//核心原因 - 函数本来就有重载的机制，这里已经达到了“类似偏特化”的效果
template<class T>
void func_partical_specialization(T, int)//更具体的实现
{

}

//函数的全特化 - 支持
template<>
void func_partical_specialization<int, double>(int, double)//更具体的实现
{
}



template<class T, typename = std::void_t<typename T::value>>
inline constexpr bool has_attri_impl(int)
{
    return true;
}

template<class>
inline constexpr bool has_attri_impl(...)
{
    return false;
}

//对外提供的版本 - 再多封装了一层
template<class T>
inline constexpr bool has_attri()
{
    return has_attri_impl<T>(0);
}

struct ABC{
    using value = int;
};

void has_attri_usage()
{
    auto b = has_attri<int>();
    auto bb = has_attri<ABC>();//需要传个参数就很鸡肋
}


//应该是把true和false封装成类型或者常量，不能封装成函数，因为函数需要运行时地调用，不符合在编译期求值
template<class T, typename = void>
struct owns_type : std::false_type{};

template<class T>
struct owns_type<T, std::void_t<typename T::value>> : std::true_type{};



void owns_type_usage()
{
    constexpr bool has_type = owns_type<ABC>::value;
    constexpr bool has_type_1 = owns_type<int>::value;
}



#endif // TEMPLATE_DEMO_HPP
