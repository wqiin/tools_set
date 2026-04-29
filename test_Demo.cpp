#include "time_operation.hpp"
#include "string_operation.hpp"
#include "general_operation.hpp"
#include "json_operation.hpp"
#include "xml_operation.hpp"
#include "filesystem_operation.hpp"
#include "custom_error_code.hpp"

#include <iostream>

#include <map>
#include <set>
#include <vector>
#include <list>
#include <thread>
#include <mutex>
#include <chrono>



//C++23标准引入
#include <flat_map>


#include "std_map_ex.hpp"

// #include "iguana/json_writer.hpp"

//#include "nlohmann/json.hpp"


void mutex_usage()
{
    std::mutex m1;
    {
        m1.lock();
        std::cout << "This is std::mutex demo";
        m1.unlock();

        {
            std::lock_guard<std::mutex> l(m1);//自动加锁，离开作用域后自动解锁
            //赋值运算符和拷贝构造被禁用
        }

        {

            {
                std::unique_lock<std::mutex> l(m1);//构造时调用 m1.lock(), 析构时自动 unlock()
                //此时没有调用l.unlock(), l仍然持有锁，不能调用lock，否则会死锁
                //可以移动构造
            }

            std::unique_lock<std::mutex> l(m1, std::defer_lock);//defer_lock告诉i构造函数，先创建锁对象，但不立即对 mutex 上锁，等到合适时机再 lock

            // 做一些不需要锁的事情

            l.lock();//延迟后，自动加锁

            {
                std::unique_lock<std::mutex> lInner(m1, std::adopt_lock);//告诉 lock：“锁已经加好了，你接管就行”

                //怎么判断能不能调用 lock()？ 使用owns_lock方法判断是否有用锁
                if(!lInner.owns_lock()){
                    lInner.lock();//不持有锁时，可以手动加锁
                }

                {
                    std::unique_lock<std::mutex> lInner(m1, std::try_to_lock);//try_to_lock告诉 lock, 在构造时，尝试加锁，
                }
            }

            l.unlock();

            l.try_lock();
        }

        //总结：std::unique_mutex在默认构造时，即std::unique_lock<std::mutex> l(m1)，会默认给m1加锁，析构时默认释放锁，
    }


    std::recursive_mutex m2;//是 C++ 里一种**“可重入锁”**，专门解决“同一线程需要多次加锁同一把锁”的问题。
    //同一个线程可以多次 lock，同样次数 unlock 才会真正释放
    //如果一个线程已经多次加锁（还没完全释放），
    //👉 其他线程调用 lock() 会被阻塞，直到锁完全释放（计数归零）
    //即，加锁的线程必须全部把锁全部释放后，另一个线程才能成功加锁，否则会阻塞
    {
        //使用场景：
        //1.成员函数互相调用，且每个函数都加锁了，
        //2.递归函数
        {
            std::lock_guard<std::recursive_mutex> lock(m2);//RAII管理可重入锁
        }

        {
            std::unique_lock<std::recursive_mutex> lock(m2);
        }

        {
            std::unique_lock<std::recursive_mutex> lock(m2, std::defer_lock);//延迟加锁
        }

        {
            std::unique_lock<std::recursive_mutex> lock(m2, std::try_to_lock);//尝试加锁
        }
    }

    std::timed_mutex m3;
    // std::timed_mutex 是在 std::mutex 基础上扩展出来的一个锁，它的核心能力是：
    // 支持“带超时”的加锁，也就是：你可以等一段时间拿锁，拿不到就放弃，而不是一直卡住。
    {
        using namespace std::chrono_literals;

        //这里的时间精度无法保证，取决于系统调度
        m3.try_lock_for(1s);
        m3.unlock();

        //auto future = std::chrono::steady_clock::now() + 3s;
        m3.try_lock_until(std::chrono::steady_clock::now() + 3s);

        //配合std::unique_lock使用
        {
            std::unique_lock<std::timed_mutex> lock(m3, std::defer_lock);//延迟加锁
            if(lock.try_lock_for(1s)){
                std::cout << "Lock the mutex successfully\n";
            }
        }

        //“如何用 try_lock_for + 退避算法 写高性能锁竞争”
    }


    std::recursive_timed_mutex m4;
    //可以看成是前面两个锁的“合体版”：recursive_mutex（可重入） + timed_mutex（带超时）
    {
        using namespace std::chrono_literals;

        m4.try_lock_for(12s);//尝试12秒，去获取锁

        {
            //推荐与std::unique_lock使用
            std::unique_lock<std::recursive_timed_mutex> lock(m4, std::defer_lock);
        }



    }

    std::shared_mutex  m5;//是 C++17 引入的一个非常重要的锁类型，用来解决：“读多写少”场景下的性能问题
    //读共享，写独占） 注意:只要当前有线程持有“读锁”，写线程就无法获取写锁
    //只要：所有读线程都释放锁,👉 写线程就能拿到锁
    //shared_mutex 是“读优先”锁，不会帮你保证写线程公平性
    //写线程必须等待所有读锁释放才能执行，如果读线程持续不断进入，就会导致写线程长期无法执行（写者饥饿）。
    {
        {
            std::shared_lock<std::shared_mutex> lock(m5);//多个线程可以同时读,又称共享锁（读锁）
        }

        {
            std::unique_lock<std::shared_mutex> lock(m5);//独占锁（写锁）
        }

        //不支持使用std::lock_guard来管理共享锁
        //std::lock_guard<std::shared_mutex> lcok_shared;

        //写者饥饿 - 写者饥饿（Writer Starvation） 是并发编程里一个非常经典的问题，尤其是在使用 std::shared_mutex 这种读写锁时很容易出现。
        //写线程一直拿不到锁，因为读线程源源不断地抢占资源

        //shared_mutex 的策略通常是：允许新的读线程继续进入，只要当前没有写锁，读线程不断插队，写线程一直被推后
        //用一句话总结机制 - 读优先（reader-preferred）策略 → 导致写者饥饿

        //一句话总结 - 写者饥饿是指在读写锁中，由于读线程不断进入，导致写线程长期无法获得锁的一种不公平现象。

        //如何解决写者饥饿？
        //1.使用“写优先”锁（最根本）
        //2.自己加控制（常见工程手段）
        //std::atomic<bool> writer_waiting = false;
        //读线程：
        //if (writer_waiting) {
        // 等待或拒绝
        //}

        //写线程：writer_waiting = true;
        //std::unique_lock lock(m);
        //writer_waiting = false;

        //3. 使用 std::shared_timed_mutex + try_lock
        //if (!m.try_lock_for(100ms)) {
        // 放弃或重试
        //}

        //4.设计层优化（最推荐🔥）
        //把结构改成：读无锁（atomic / copy-on-write）、写少且集中
    }

    std::shared_timed_mutex m6;
    //是 C++14 提供的一个读写锁类型，可以看成：std::shared_mutex（多读单写） + timed_mutex（支持超时）
    //所以它适用于：读多写少 + 不希望写线程卡死
    {
        using namespace std::chrono_literals;
        m6.lock_shared();//阻塞获取读锁
        m6.unlock_shared();//释放读锁

        m6.try_lock_shared();//尝试获取读锁
        m6.unlock_shared();//释放读锁

        m6.try_lock_shared_for(1s);
        m6.unlock_shared();//释放读锁

        //获取独占的写锁
        m6.lock();
        m6.unlock();

        m6.try_lock_for(100ms);//尝试获取独占的写锁
        m6.unlock();

        //配合std::shared_lock使用

        {
            std::shared_lock<std::shared_timed_mutex> lock(m6);//获取读锁
        }

        {
            std::unique_lock<std::shared_timed_mutex>lock(m6, std::defer_lock);

            lock.try_lock_for(100ms);

            lock.unlock();
        }

    }

    std::condition_variable cv1;
    //是 C++ 并发里一个非常核心的同步工具，它解决的不是“互斥”，而是：线程之间的“等待 + 通知”问题
    //std::condition_variable，只能与std::unique_lock<std::mutex>配合使用

    {
        using namespace std::chrono_literals;
        std::unique_lock<std::mutex> lock(m1);
        bool bReady = false;

        //wait 之前：线程必须已经持有 mutex 锁， lock必须获得该锁

        //在wait时，做了三件事 - (会检查条件是否满足，不满足才会继续执行后续的三件事)1. 释放锁 2. 线程睡眠 3. 被唤醒后重新加锁，再检查条件
        //为什么“第三步，必须再次检查条件”？ - 防止虚假唤醒；多个线程竞争(多个线程等待，当前被虚假唤醒)；条件可能被“抢走”
        //也就是：“醒来之后必须再判断一次”

        //wait_for不会丢通知的原因（重点🔥）
        //1. 把线程加入等待队列 2. 释放 mutex 3. 睡眠 ， 这三步为原子操作，不会被中断
        //wait 会在锁保护下检查 ready
        //“为什么 wait 必须释放锁？如果不释放会怎样？” - wait 必须释放锁，否则其他线程无法修改条件，等待线程将永远无法被唤醒（死锁）

        cv1.wait_for(lock, std::chrono::milliseconds(100), [&bReady]{
            return bReady;
        });

        std::jthread([&m1, &bReady, &cv1]{
            std::this_thread::sleep_for(1000ms);

            {
                std::unique_lock<std::mutex>l(m1);
                bReady = true;
                //这里修改判断条件时，如果不使用锁，而直接修改条件，可以导致丢失通知
                /*
                 * T1: consumer 拿锁
                T2: consumer 判断 ready == false ✅

                ---- 切换线程 ----

                T3: producer 执行
                    ready = true//这里没有获取锁，直接修改判断条件，如果获取了，则会阻塞在这里，就不会丢失通知
                    notify_one()

                ---- 切回 consumer ----

                T4: consumer 调用 wait() → 释放锁 → 进入睡眠 😴

                本质上就是，另一个线程还没睡眠，就被提前通知了，就因为没有，两边没有同步，导致通知丢失

                mutex 的作用是：把“检查条件”和“进入等待”变成一个不可分割的临界区
                 * */
            }

            //这里不需要持有锁就可以唤醒
            //它只是一个“通知机制”，不会访问临界资源，自然不需要加锁：唤醒一个正在 wait 的线程；真正需要同步的是“条件状态”，而这个由 mutex 来保护。
            //它不关心：ready 是多少,条件是否满足
            cv1.notify_one();
        });
    }


    std::condition_variable_any cv2;
    //相比于std::condtion_variable，适配所有类型锁，且支持自定义锁, 即std::unique_lock<MUTEX_T>, MUTEX_T为任意类型的锁
    //只要该锁实现了lock和unlock方法就行
    {
        using namespace std::chrono_literals;
        std::unique_lock<std::recursive_mutex> lock(m2);
        cv2.wait_for(lock, 200ms, []{
            return true;
        });

    //std::condition_variable_any 是一个更通用但性能略低的条件变量，允许与任意符合 Lockable 概念的锁配合使用，而 std::condition_variable 更高效但只支持 std::mutex。
    }


    //为什么 shared_mutex + condition_variable 很难正确配合？
    //一句话结论 - ❗std::shared_mutex + std::condition_variable 很难正确配合，因为
    //condition_variable 要求“独占锁语义”，而 shared_mutex 存在“共享锁语义” → 不匹配
    //CV只会调用lock和unlock方法，这对于读写锁来说，读和写都是独占了，那么共享锁就失去了意义了
    //读写锁，直接退化为了std::mutex

    //shared_mutex 不适合直接和 condition_variable 搭配，因为条件变量依赖“独占锁语义”，而 shared_mutex 的共享读锁会破坏这种语义，甚至导致死锁或条件无法推进。
    //推荐std::recursive_mutex + std::condition_variable_any


    //std::conditon_variable的进阶用法




    //无锁编程//
    //无锁编程 = 不使用 mutex，而用原子操作（atomic）来实现线程安全
    //无锁编程是通过原子操作（尤其是 CAS）来实现线程安全的一种并发模型，避免了锁带来的阻塞和开销，但代价是实现复杂度高且容易出错。

    /*
     * std::atomic<T>, 中但 T 必须是“平凡可复制（trivially copyable）”的类型
     *不满足的自定义类型；
     *1. 含有自定义构造/析构
     *2. 含有 std::string
     *3. 含有虚函数
     *如何判断一个类型是否可以？ 用这个：std::is_trivially_copyable_v<T>
     *如，static_assert(std::is_trivially_copyable_v<Data>);
     *
     *为什么有这个限制？ std::atomic 本质是：直接按内存位拷贝（bitwise copy），即浅拷贝
     */
}


void test()
{
    std::cout << Helper::get_current_format_datetime() << std::endl;;
    std::cout << Helper::get_timespamp() << std::endl;

    std::cout << Helper::string::format("Hello %s - %d", "world", 53) << std::endl;;

    std::uint8_t n8 = 11;
    std::uint16_t n16 = 16;
    std::uint32_t n32 = 32;
    std::uint64_t n64 = 0;



    auto result = Helper::general::make_uint<std::uint16_t, std::uint8_t>(n8, n16);
    std::cout << result << std::endl;

    std::vector<int> vec{1, 2, 3, 4};
    std::set<int> set{1, 2, 3, 4};
    std::map<int, int> map{{1, 1}, {2, 2}, {3,3}, {4, 4}};
    std::list<int> lst{1, 2, 3, 4};

    auto pVec = Helper::general::find_if_as_ptr(vec, [](int n){
        return n > 2;
    });
    std::cout << "pVec: " << *pVec << std::endl;

    auto pSet = Helper::general::find_if_as_ptr(set, [](int n){
        return n > 1;
    });
    std::cout << "pSet: " << *pSet << std::endl;

    auto pMap = Helper::general::find_if_as_ptr(map, [](std::pair<int ,int> pa){
        return pa.first > 3;
    });
    std::cout << "pMap: " << pMap->first << std::endl;

    auto pLst = Helper::general::find_if_as_ptr(lst, [](int n){
        return n > 0;
    });
    std::cout << "pLst: " << *pLst << std::endl;

    //不能传入右值，只能传入左值
    // auto pVecRight = Helper::general::find_if_as_ptr(std::vector<int>{1, 2, 3, 4}, [](int n){
    //     return n > 2;
    // });
    // std::cout << "pVec: " << *pVec << std::endl;

    auto func = [](){
        return 0;
    };

    bool b = func;
    if(func){
        std::cout << "not Empty" << std::endl;
    }else{
        std::cout << " Empty" << std::endl;
    }

    //这里为原地构造，为C++17标准规定的行为，而不是调用了赋值构造方法，这里也没调用移动构造
    //C++17的强制拷贝省略
    Helper::general::scope_guard hello = Helper::general::scope_guard(func);

    //这里调用的是移动构造，原因 - hello2为一个新构造的对象，关键是看新构造对象还是操作原有的对象
    //即使这里写了 '=', 由于hello2为一个新构造的对象，这里所以是调用移动构造，输出的日志也证明这一点
    //关键是看  - 有没有“已经存在的对象”， 所以，这里能够正常编译
    auto hell02 = std::move(hello);

    //禁用了赋值运算法，这里编译会报错
    //auto hello1 = hello;

    int * p = nullptr;
    Helper::general::safe_delete(p);

    Helper::general::function_map<int, std::string, int, int> fmp;
    fmp.register_func("plus", [](int a, int b){
        return a * b;
    });

    fmp.register_func("add", [](int a, int b){
        return a + b;
    });

    auto retOpt1 = fmp.run_functor("add", 2, 3);
    auto retOpt2 = fmp.run_functor("plus", 2, 3);

    std::cout << "add result: " << *retOpt1 << std::endl;
    std::cout << "plus result: " << *retOpt2 << std::endl;

    {
        Helper::general::sync_map<std::string, int> syncMp;
        std::thread t1([&]{
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(500ms);
            syncMp.set_value("Hello", 53);
        });

        std::thread t2([&]{
            auto status = syncMp.wait_for(1000, "Hello", [](int v){
                bool bRet = v >= 53;
                return bRet;
            });

            if(status == Helper::general::ENWaiterStatus::En_success){
                std::cout << "Sync map waitting the sync result being OK\n";
            }else{
                std::cout << static_cast<int>(status);
            }
        });

        t1.join();
        t2.join();

        std::jthread jth;
    }
}


void test1()
{
    // Helper::general::MapEx<int, int, std::mutex, std::map> mp{{1, 1}, {2, 2}, {3, 3}};

    // auto keys = mp.get_keys<std::list>();
    // for(auto key : keys){
    //     std::cout << "Keys: " << key << std::endl;
    // }

    // mp.for_each([](const int & k, int & v){
    //     v += 1;
    // });

    // mp.for_each([](const int & k, int & v){
    //     std::cout << "key:" << k << " value:" << v << std::endl;
    // });

    // std::cout << "is contains keys: " << mp.contains(1) << std::endl;

    // //这样，在遍历的时候，可以一直持有锁
    // // auto view = mp.keys_view();
    // // for(auto key : view){
    // //     std::cout << "key---->" << key << std::endl;
    // // }

    // std::cout << "Stuck here befoe1\n" << std::endl;;
    // for(auto key : mp.get_keys<std::vector>()){
    //     std::cout << "Keys: " << key << std::endl;
    // }

    //std::cout << "Stuck here befoe1\n";
    //auto view1 = mp.keys_view();
    std::cout << "Stuck here befoe\n";

    std::atomic<int> at{23};

    Helper::map::vector_map<int, int> vec_map;
    vec_map.push_back(1, 1);
    vec_map.push_back(2, 2);


    vec_map.for_each([](int key, std::vector<int> & vals){
        std::cout << "keys: " << key << "_____";

        std::cout << "values: ";
        for(auto & val : vals){
            std::cout << val;
        }
        std::cout << "---------------------------";
    });

    Helper::map::MapEx<int, std::shared_ptr<std::string>> mp{};

}

void test2()
{
    Helper::time_elapsed timer;
    using namespace std::chrono_literals;

    // std::this_thread::sleep_for(3000ms);
    // std::cout << "time elapsed for " << timer.elapsed_time() << std::endl;

    // //重新开始计时
    // timer.reset();
    // std::this_thread::sleep_for(3000ms);
    // std::cout << "time elapsed for " << timer.elapsed_time<std::chrono::microseconds>() << std::endl;

    // //重新开始计时
    // timer.reset();
    // std::this_thread::sleep_for(3000ms);
    // std::cout << "time elapsed for " << timer.elapsed_time<std::chrono::seconds>() << std::endl;

    // timer.reset();
    // std::this_thread::sleep_for(3000ms);
    // std::cout << "time elapsed for " << timer.elapsed_time<std::chrono::nanoseconds>() << std::endl;

    auto paRet = Helper::time_elapsed::measure([](int a)->std::string{
        std::this_thread::sleep_for(3000ms);
        std::cout << a << std::endl;
        return std::string{"Hello World"};
    }, 53);

    std::cout << "Ret Value: " << paRet.first << " elapsed time for:  " << paRet.second << std::endl;

    auto ret = Helper::time_elapsed::measure([]{
        std::this_thread::sleep_for(3000ms);
    });

    std::cout << "elapsed time for:  " << ret.second << std::endl;
}


//结构体的变量名称需要与格式化字符串中的字段名称一致
struct Student
{
    std::string name;
    int nAge;
    std::string address;
    std::string father_name;
    int grade;
    std::string mother_name;
};

struct some_obj {
    std::string_view name;
    int age;
};
YLT_REFL(some_obj, name, age);


void test3()
{
    auto ret =  Helper::string::join_string(' ', "Hello", "World", "!");
    std::cout << ret << std::endl;

    const char * js = R"({"name":"Hola", "nAge":23, "address":"SC China", "father_name":"Hello", "grade":12, "mother_name":"Nihao"})";

    Student stu;
    std::string strErrMsg;
    auto b = Helper::json::json_2_obj(std::string(js), stu, &strErrMsg);
    if(!b){
        std::cout << strErrMsg << std::endl;
    }

    std::cout << "Name: " << stu.name << std::endl;

    Student stu_out;
    auto ret_js = Helper::json::obj_2_json(stu, true);
    std::cout << "Ret json: " << ret_js << std::endl;



    std::string_view xml = R"(
    <!-- Hello world -->
    <root>
      <name>buke</name>
      <age>30</age>
    </root>
    )";

    some_obj obj;
    auto ret_xml = Helper::xml::xml_to_obj<some_obj, std::string_view>(xml, obj, &strErrMsg);
    if(!ret_xml){
        std::cout << "xml decode error message: " << strErrMsg << std::endl;
    }else{
        std::cout << "xml decode: " << obj.age << "-------" << obj.name << std::endl;
    }

    //这里返回 - <some_obj><name>buke</name><age>30</age></some_obj>
    auto str_ret_xml = Helper::xml::obj_to_xml(obj);
    std::cout << "xml from string: " << str_ret_xml << std::endl;


    std::error_code ec = CustomErrCode::OK;

}


void test4()
{
    std::cout << "current_path: " << Helper::fs::get_current_path() << std::endl;

    std::cout << "current_path: " << Helper::fs::absolute_path("/HEllo") << std::endl;

    Helper::fs::create_directories("./Hello/World");//在当前程序运行路径下，递归创建目录

    Helper::fs::rename_directory("./Hello", "./Hola");

    std::function<int()> lambda = []()->int
    {

    };
    using TT = std::decay_t<decltype(lambda)>;
    TT tt;

    using T = std::decay_t<decltype(test1)>;
    T a;

    const volatile int && int_cv = 32;
    using ORI_CVT = decltype(int_cv);//得到被CV和引用修饰的原始类型
    using CVT = std::decay_t<decltype(int_cv)>;//去除CV和引用限定符
    CVT cvt;
    ORI_CVT ori_cvt = 23;

    //remove_cvref_t与std::decay_t完全一样
    using RMCVREF = std::remove_cvref_t<ORI_CVT>;
    RMCVREF rmcvref = 23;

    using INT_ARR = int[10];
    using A = std::decay_t<INT_ARR>;//A已经退化为指针
    using B = std::remove_cvref_t<INT_ARR>;//还是int的数组

    using C = std::unwrap_ref_decay_t<INT_ARR>;
    using D = std::unwrap_reference_t<INT_ARR>;

    //std::ref();

    //变相在容器中存放‘引用’
    int na = 2;
    int nb = 2;
    std::vector<std::reference_wrapper<int>> vec_ref;
    vec_ref.push_back(na);
    vec_ref.push_back(std::ref(nb));//把变量 x 包装成一个“可以复制的引用”（std::reference_wrapper）
    vec_ref.at(0).get() = 23;//访问引用包装器中的原始引用

    {
        namespace fs = std::filesystem;
        //使用std::ref向线程中传入变量引用
        auto lam = [](int & x){
            x = 23;
        };

        //std::ref->std::reference_wrapper, 且std::reference_wrapper重载了operator T&() const， 所以会隐式转换;
        std::jthread j(lam, std::ref(na));

        //抑或是
        auto g = std::bind(lam, std::ref(nb));
        g();
    }


    std::unwrap_ref_decay_t;//用于向模板函数中传入引用变量时，在模板中得到引用类型，同时去除CV修饰
    std::unwrap_reference_t;//用于向模板函数中传入引用变量时，在模板中得到引用类型，即从std::reference_wrapper<T>得到T&类型

    A aaa;
    B bb;

}


// template<class L, class R>
// auto hello_lambda(L && left, R && right, Func_t<T(L, R)> & func)
// {
//     return std::invoke(func, std::forward<L>(left), std::forward<R>(right));
// }

template<class T>
auto print_items = [](std::span<const T> data)//const限制修改span中的元素
{
    int nIndex = 0;
    for(const auto & item : data)
    {
        std::cout << "item " << nIndex++ << " :" << item << std::endl;
    }
};

#include "pprint.hpp"
#include <algorithm>

#include "template_demo.hpp"
void test5()
{
    auto cur_tp = Helper::get_timespamp<std::chrono::milliseconds>();
    std::cout << cur_tp << std::endl;

    int inta = 23;
    int intb = 32;

    std::swap(inta, intb);


    std::vector<int> vecInt{1, 2, 3, 4, 5, 6, 7};
    int arr[] = {5, 4, 3, 2, 1, 0};
    print_items<int>(vecInt);
    print_items<int>(arr);//会自动推导为std::span<int>

    std::vector<std::string> vecStr{"Hello", "World", "Hola", "Nihao"};
    print_items<std::string>(vecStr);

    std::span s(arr);//拷贝构造
    std::span s1 = arr;//复制构造



    auto front_3 = s.first<2>();//用模版访问切片
    auto front_3_ = s.first(2);//用参数访问切片

    print_items<int>(front_3);
    print_items<int>(front_3_);



    auto even = [](int n){ return n % 2 == 0;};

    //惰性执行（lazy）！, view在使用的时候，才会去计算 运算符 | ;
    std::ranges::filter_view<std::ranges::ref_view<std::vector<int>>, decltype(even)> view = vecInt | std::views::filter(even);// | std::views::transform([](int x){return x * x;});
    //std::ranges 是“更安全、更强大、更可组合”的 STL 算法与容器接口体系

    //这里std::views::transform只会在访问view1的时候才会去访问vecInt,并根据views计算得到结果
    //lazy evaluation（惰性执行）
    auto view1 = vecInt | std::views::filter(even) | std::views::transform([](int x){return x * x;});

    auto it = std::ranges::find_if(vecInt, even);

    //std::ranges::range_reference_t<int>;


    pprint::PrettyPrinter prt(std::cout);
    prt.quotes(true);//在打印字符串时，加上引号
    prt.compact(true);//紧凑打印
    prt.print(vecStr);

    auto ret = Helper::time_elapsed::measure<std::chrono::seconds>([](){
        std::cout << "Hello World\n";
        std::this_thread::sleep_for(std::chrono::seconds(1));
    });

    std::cout << "last for ;" << ret.second << std::endl;

    std::string str = "123";
    std::string str_ret;

    auto ret_int = Helper::string::string_to_number<long int>(str);
    std::cout << "ret int: " << ret_int.value_or(0) << std::endl;

    std::function<std::string(int, int)> hola = [](int a, int b)->std::string
    {
        return std::to_string(a) + ":    " + std::to_string(b);
    };

    //std::cout << hello_lambda<int, int, std::string>(23, 32, hola);
}


template<class Func_t, class...Args>
auto safe_calling(Func_t && fun, Args&& ...args)
{
    if constexpr(std::is_invocable_v<Func_t&&, Args&&...>){
        return std::invoke(std::forward<Func_t>(fun), std::forward<Args>(args)...);
    }else{
        return nullptr;
    }
}

void test6()
{

    auto local = [](int a, float f)->std::string
    {
        std::cout << "Hello World from local lambda\n";
        std::cout << "a = " << a << " f=" << f << std::endl;

        return "Return value";
    };

    auto local_void = []->void{
        std::cout << "Lambda without return vale\n";
    };

    auto ret = safe_calling(local, 23, 32.0f);
    std::cout << "Ret val:" << ret << std::endl;

    safe_calling(local_void);

    // float f1 = 1.00000001;
    // float f2 = 1.00000002;

    // std::cout << "float equal: " << is_equal(f1, f2);


    // array<int, 5> arr1;
    // std::cout << "array size: " << arr1.size() << std::endl;
    // arr1[1] = 2;


     std::vector<int, JJ::allocator<int>> vec{1, 2, 3, 4};
    // std::array<int, 3> arr;

    // //std::vector<std::string> vec_str;

    // std::list<std::string> lst;

    // //std::find();//本质上就是一个循环遍历的结果

    // //std::alloc;

    // for(auto item : vec){
    //     std::cout << item;
    // }

    // func(12.0);

    // std::queue<int> que;

    // std::source_location loc = std::source_location::current();//
    // std::cout << "file name: " << loc.file_name() << "  line id: " << loc.line() << "  func name: " << loc.function_name() << std::endl;

    // add(2, 3);
    // add(2.3, 3.0);

    //concept in lambda
    auto f = []<integral T>(T a){
        return a * a;
    };

    std::cout << f(3) << std::endl;


    default_val<int> dv1;
    default_val<std::uint32_t> dv2;
    default_val<std::string> dv3;
    default_val<float> dv4;
    default_val<double> dv5;

}