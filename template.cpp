

//C++20
#include <span>
#include <ranges>
#include <functional>
#include <future>

#include <algorithm>
#include <math.h>
#include <limits>
#include <concepts>



/*
// ==================== C++11 ====================
template<typename T,
        typename std::enable_if<std::is_integral<T>::value, int>::type = 0>
T add(T a, T b){
    return a + b;
}

// ==================== C++14 ====================
template <typename T, std::enable_if_t<std::is_integral<T>::value, int> = 0>
T add(T a, T b) {
    return a + b;
}

// ==================== C++17 ====================
// _v 后缀来了，但签名约束还是 enable_if
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T add(T a, T b) {
    return a + b;
}

// ==================== C++20 ====================
template <std::integral T>
T add(T a, T b){
    return a + b;
}

// 或者简写（a和b类型独立推导，可以不同）：
auto add(std::integral auto a, std::integral auto b) {
    return a + b;
}
*/




//





