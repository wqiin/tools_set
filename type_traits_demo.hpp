#ifndef TYPE_TRAITS_DEMO_HPP
#define TYPE_TRAITS_DEMO_HPP

#include <type_traits>
#include <functional>//std::invoke
#include <cstdint>
#include <string>
#include <utility>//std::declval






int common_func(int )
{
    return {};
}

void decltype_usage()
{
    using FuncPointer = decltype(common_func);
    FuncPointer funcP;

    //common_func 是一个：函数名 / 函数对象 / lambda变量,它是一个值（value,而模板参数这里需要的是类型（type）
    constexpr bool is_common_func_being_invocable = std::is_invocable_v<decltype(common_func), float*>;
    constexpr bool is_common_func_being_invocabl_1 = std::is_invocable_v<decltype(common_func), float>;//隐式转换到int，可以调用

    auto local_lambda = [](float f)->int
    {
        return {};
    };

    constexpr bool is_local_lambda_being_invocable = std::is_invocable_v<decltype(local_lambda), int>;//隐式转换到float，可以调用

    //验证第二个模板参数和其后参数(如果有的话)是否可以调用，并且，判断调用的返回值是否与第一个模板参数一致
    constexpr bool is_invocalbe_and_valid_return_value = std::is_invocable_r_v<int, decltype(common_func), int>;


    //这里加上了 &限定符，表示重载的括号运算符只能让左值调用
    struct F {
        void operator()() & {}   // 只能左值调用
    };


    auto template_in_local_lambda = []<class Func, class ...Args>(Func && f, Args&&... args){

        //这里加上&&，使得传入的可调用对象的左右值都可以调用，使用引用折叠
        static_assert(std::is_invocable_v<Func&&, Args&&...>, "Func Must be callable with given arguments.");

        return std::invoke(std::forward<Func>(f), std::forward<Args>(args)...);
    };


    struct AA{
        AA(int);//没有默认构造函数

        int foo() const;//{return 0;};
    };
    //std::declval 是一个只在编译期用的工具函数，核心作用一句话：👉 “假装有一个某种类型的值，用来做类型推导（但并不会真的创建对象）”。
    //using Decl_t0 = decltype(AA().foo()); // ❌ 编译错误（AA() 不存在）

    //std::declval<AA>()的作用是假设使用AA的默认构造实现一个对象，然后再进行后续的类型推导
    using Decl_t = decltype(std::declval<AA>().foo());//int,得到假装调用foo的返回值类型，这里为int



    struct CC{
        virtual ~CC(){};

        virtual void foo() = 0;
    };
    constexpr bool is_abstract = std::is_abstract_v<CC>;//判断一个类是否为抽象类


    struct DD{
        int a;
        float cc;
        double dd;
    };

    //std::alignment_of 是一个类型萃取，用来获取：👉 某个类型的“对齐要求（alignment requirement）”
    constexpr auto align = std::alignment_of_v<DD>;

    constexpr auto align_1 = alignof(DD);//基本等价于std::alignment_of_v


    struct EE final{
    };

    //std::is_final 是一个类型萃取，用来判断：👉 一个类是否被 final 修饰（也就是“不能再被继承”）。
    auto is_final = std::is_final_v<EE>;

    //底层实现
    /*
     *  template <class _Tp>
        inline const bool __is_null_pointer_v = __is_same(__remove_cv(_Tp), nullptr_t);
     */
    std::nullptr_t np = nullptr;
    constexpr bool being_nullptr = std::is_null_pointer_v<std::nullptr_t>;//仅在传入的类型为std::nullptr_t时，为真

    struct FF{
        int a;
        int f;
        float cc;
    };

    //std::is_aggregate 用来判断：👉 一个类型是不是“聚合类型（aggregate type）”。
    /*什么是“聚合类型”？
    * 简单理解一句话：👉 可以用花括号 {} 直接按成员顺序初始化的类型
    *
    * 一个类型通常是 aggregate，需要满足：
    👉 没有这些“复杂特性”：
    ❌ 没有用户声明的构造函数（包括 =default / =delete 也算声明过）
    ❌ 没有 private / protected 非静态成员（C++17 前严格，之后规则略放宽但仍建议避免）
    ❌ 没有虚函数
    ❌ 没有虚继承
    ❌ 没有基类（C++17 之前；C++17 之后允许简单基类，但规则仍较严格）
    */

    constexpr bool is_aggregate = std::is_aggregate_v<FF>;//true



}

//聚合类的构造使用
template<typename T>
T make() {
    if constexpr (std::is_aggregate_v<T>) {
        return T{}; // 聚合初始化
    } else {
        return T(); // 构造函数
    }
}


void type_relations()
{
    // auto is_same_t = []<class T, class U>() constexpr{
    //     return std::is_same_v<T, U>;
    // };

    constexpr bool is_same_type = std::is_same_v<int, std::int32_t>;

    struct Base{};
    struct Drived : public Base{};
    struct Other{};

    constexpr bool is_drived_from = std::is_base_of_v<Base, Drived>;
    constexpr bool is_drived_from_1 = std::is_base_of_v<Base, Other>;

    constexpr bool is_convertible = std::is_convertible_v<float, int>;//true
    constexpr bool is_convertible_1 = std::is_convertible_v<float, std::string>;//false


    //std::make_signed 是一个类型变换工具（type trait），作用可以一句话概括：👉 把一个整数类型，转换成“对应的有符号版本”。
    using Signed_t = std::make_signed_t<std::uint32_t>;
    Signed_t helloWorld = 23;

    enum Color : std::int32_t{
        Green,
        Red,
    };

    //std::underlying_type_t 也是一个类型萃取工具，它的作用可以一句话说清：👉 获取“枚举类型（enum）在底层实际使用的整数类型”。
    using Enum_t = std::underlying_type_t<Color>;
    Enum_t enum_t = 23;


    //std::common_type_t<Ts...> 的作用是：找出一组类型的“共同类型”（common type。 即，在传入的多个类型中找到一个可以容纳其他所有类型的类型，
    using Common_t = std::common_type_t<char, std::int64_t>;//std::int64_t
    using Commont_t1 = std::common_type_t<int, double>; // → double
}


void pod_usage()
{
    //纯数据的结构体
    struct A{
        int a;
        float f;
        std::string str;
    };

    constexpr bool is_pod = std::is_standard_layout_v<A>;//true

    //有虚函数，不满足标准内存布局，因为加入了虚函数表指针
    struct B{
        virtual ~B(){}
    };
    constexpr bool is_pod_1 = std::is_standard_layout_v<B>;//false


    //有private或者protected修饰的数据成员
    struct C{
        int a;

    protected:
        float f;
    };
    constexpr bool is_pod_2 = std::is_standard_layout_v<C>;//false


    struct D{
        int aint;
        int bint;
    };

    //多重继承也不满足
    struct E : public D, public A{
    };
    constexpr bool is_pod_3 = std::is_standard_layout_v<E>;//false

    //非标准布局基类 👉 如果父类本身不满足，那子类也不行
    struct F : public B{};
    constexpr bool is_pod_4 = std::is_standard_layout_v<F>;//false

    //为什么这个 is_standard_layout_v 很重要？👉 在这些场景特别关键：✔ 场景1：和 C 语言交互; 场景2：序列化 / 网络传输; 场景3：底层优化 / 内存 hack
    //总结一句话 👉 std::is_standard_layout 判断一个类型的内存布局是否“像 C struct 一样规整可靠”。



    //是否是平凡类型
    //如果一个类型是 trivial，基本意味着：
    //👉 它“像 C 里的普通数据一样”，没有任何复杂的构造/析构行为
    //创建它 ≈ 不做额外工作
    //销毁它 ≈ 不做额外工作
    //可以直接按字节处理（在很多场景下）

    /*
    std::is_trivial<T> 为 true，当且仅当T满足：
        有 平凡默认构造函数
        有 平凡拷贝构造函数
        有 平凡移动构造函数
        有 平凡拷贝赋值
        有 平凡移动赋值
        有 平凡析构函数

        并且：
        没有虚函数
        没有虚继承

        并且:
        不能有自定义的构造和赋值方法

        并且：
        所有成员也应该是平凡类
    */

    struct AA{
        int a;
        int b;

    };

    struct BB{
        BB() = default;
    };

    struct CC{
        CC(){};
    };

    constexpr bool is_trivial = std::is_trivial_v<AA>;//true
    constexpr bool is_trivial_1 = std::is_trivial_v<A>;//false, 所有成员也应该是平凡类

    constexpr bool is_trivial_2 = std::is_trivial_v<BB>;//true
    constexpr bool is_trivial_3 = std::is_trivial_v<CC>;//false 不能有自定义的构造函数
    //std::is_trivial_v与std::is_standard_layout 的区别
    //trait	关注点
    //is_trivial            生命周期是否“无操作”
    //is_standard_layout	内存布局是否规整

}

/*
 * decltype(a, b)
先计算 a
再计算 b
整个表达式结果是：b

expr1 必须是合法表达式！
*/
template<typename T>
using has_size_t = decltype(std::declval<T>().size(), std::true_type{});

void decltype_usage_1()
{
    int x = 1;
    int & refX = x;


    //decltype想同事获取变量的声明类型和表达式的真实类型
    decltype(x + 1) xx = 23;//int 获得了变量的声明类型 - 特殊规则
    decltype((x)) xxx = x;//int & 获取了表达式的真实类型 - 普通规则(即表达式规则)
    decltype(std::move(x)) y = 23;//int &&，move后得到的是一个将亡值

    //引用折叠是一个编译期间的概念，而非运行时，
    //右值引用是一个运行时的概念，而非编译期

    int a = 23;
    int && rRef = 23;


    //解释为啥New_t为啥是int&
    //rRef虽然是左值，但是作为表达式的时候，其是一个有名字的变量，即是一个右值，即推导出int&
    using New_t = decltype(a, rRef);;//hello = x;//int &
    using NewrRef_t = decltype(rRef);//int&&

}

//为什么移动构造最好是noexcept
//👉 移动构造函数最好标记 noexcept，因为 STL 容器会依赖它决定“搬迁元素时用 move 还是 copy”。
//如果 move 可能抛异常，很多容器会：👉 放弃 move，改用 copy。


//为什么 C++ 要有“引用折叠”规则？
//引用折叠（reference collapsing）是为了让模板、完美转发、类型推导能够统一工作。
//C++ 语法本来不允许引用的引用。所以标准必须规定：当模板推导生成“引用套引用”时，该怎么化简。



//std::forward<T>(x) 本质确实是：static_cast<T&&>(x)但它真正关键的地方在于：T 保留了模板推导时的引用信息，从而能够正确恢复“原始值类别”。
//而，std::move 永远：tatic_cast<T&&> 👉 无条件右值化。



//下面代码为什么非法：auto x = std::declval<int>();
//declval<T>() 的核心作用是：🔥 “伪造一个 T 类型表达式，用于类型推导” 它不是为了真正创建对象。

//把编译期间的虚假类型推导，放在了运行期间；因此其只能用于“未求值上下文（unevaluated context）”，即编译期间

//std::declval<A>() 即使：A： 不可构造 构造 deleted抽象类 也能“假装存在”。





#endif // TYPE_TRAITS_DEMO_HPP
