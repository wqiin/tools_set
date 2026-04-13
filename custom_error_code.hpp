#ifndef CUSTOM_ERROR_CODE_HPP
#define CUSTOM_ERROR_CODE_HPP


#include <system_error>

//自定义的错误码系统
enum class CustomErrCode
{
    OK = 0,
    FileNotExist,
    InvalidParams,
};

class CustomCategory : public std::error_category
{
public:
    const char * name() const noexcept override{
        return "Custon Error Class";
    }

    std::string message(int ev) const override
    {
        switch(static_cast<CustomErrCode>(ev)){
        case CustomErrCode::FileNotExist:
            return "File Not Exists";

        case CustomErrCode::OK:
            return "OK";

        case CustomErrCode::InvalidParams:
            return "Invalid Parameters";
        }
    }
};

std::error_code make_error_code(CustomErrCode ec){
    static CustomCategory cat;
    return {static_cast<int>(ec), cat};
}

namespace std{
template<>
struct is_error_code_enum<CustomErrCode>:true_type{};
}

#endif // CUSTOM_ERROR_CODE_HPP
