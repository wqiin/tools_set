
#include <folly/FBString.h>
#include <folly/container/F14Map.h>
#include <folly/small_vector.h>
//#include <folly/concurrency/ConcurrentHashMap.h>

#include <folly/sorted_vector_types.h>
//#include <folly/dynamic.h>

//#include <folly/io/IOBuf.h>

#include <folly/memory/Arena.h>//一个高性能内存池

#include <iostream>
#include <string>

class CustomNumberic{
private:
    int age = 0;
    float grade = 0.0f;
};

void folly_map()
{
    using namespace folly;

    F14FastMap<std::string, int> fast_mp{{"Hello", 1}, {"Hola", 2}, {"Nihao", 3}};
    for(auto & [k, v] : fast_mp){
        std::cout << "key: " << k << "  value:" << v << std::endl;
    }


    //F14FastMap fast_mp_1{{"Hello", 1}, {"Hola", 2}, {"Nihao", 3}};
}

void folly_small_vector()
{
    folly::small_vector<int, 8> sm_vec{0};
    sm_vec.push_back(1);

    auto size = sm_vec.size();

}

void folly_concurrent_hasp_map()
{
    //folly::ConcurrentHashMap<int, int> chp;//{{1, 2}, {2, 3}};;

    //该容器使用hash算法，对key映射一个对应的atomic原子变量，使用其来互斥读写，对同一个key-value的读写，只会占据一个锁，对其他的key-value不会跟其他的产生数据竞争

    // chp.emplace(1, 2);
    // chp.assign_if()
}


void folly_sorted_vector_map()
{
    folly::sorted_vector_map<int, std::string> svp;

    //本质上为一个 class Container = std::vector<std::pair<Key, Value>, std::allocator<std::pair<Key, Value>>>>， 只是保持有序
}

void folly_dynamic()
{
    // folly::dynamic v;

    // v = 123;
    // v = "hello";
    // v = true;

    // folly::dynamic obj = folly::dynamic::object
    //     ("name", "Tom")
    //     ("age", 18);


    // folly::dynamic user =
    //     folly::dynamic::object("name", "Alice") ("age", 20)
    //     ("skills",folly::dynamic::array("C++","Python","Rust"));

    // std::cout
    //     << user["name"].asString()
    //     << std::endl;

    // for (auto& s : user["skills"])
    // {
    //     std::cout
    //         << s.asString()
    //         << std::endl;
    // }
}

void folly_io_buf()
{

}


void folly_usage()
{
    std::string::npos;
    //alignas(CustomNumberic) 修饰后面的buffer，使其满足CustomNumberic对其要求，类似于const和static
    alignas(CustomNumberic) char buffer[251] = {0};

    folly_map();

    auto len = sizeof(long);

    folly::fbstring s = "hello";
    std::cout << s << std::endl;
}