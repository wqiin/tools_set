#ifndef GENERAL_DEFINE_HPP
#define GENERAL_DEFINE_HPP

#include <cstdint>

namespace Helper
{
    namespace emun_definition
    {
        enum class ENWaiterStatus : std::uint32_t
        {
            En_success = 0,     //成功
            En_timeout,          //超时
            En_interrupt,       //中断，即被终止或者取消
            En_invalid,         //无效值
        };


    }
}


#endif // GENERAL_DEFINE_HPP
