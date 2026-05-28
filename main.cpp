
extern void test();
extern void test1();
extern void test2();
extern void test3();
extern void test4();
extern void test5();
extern void test6();

extern void event_bus_demo();

extern void folly_usage();
//extern void type_erase_usage();


#include <functional>
#include <iostream>

template<class T, class...Args>
using callable_ = std::function<T(Args...)>;


#include "type_eraser.hpp"

int main(int argc, char *argv[])
{
    //event_bus_demo();

    // std::function<void()> func = nullptr;
    // std::cout << "std::functional size: " <<sizeof(func) << std::endl;

    //folly_usage();

    //test6();

    type_erase_usage();

    return 0;    
}
