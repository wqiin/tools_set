#ifndef JSON_OPERATION_H
#define JSON_OPERATION_H

#include <string>
#include <memory>

#include "iguana/json_writer.hpp"
#include "iguana/json_reader.hpp"
#include "iguana/prettify.hpp"

//#include "bson_decoder.h"
//#include "bson_encoder.h"

//#include "nlohmann/json.hpp"


namespace Helper {
namespace json {

    template<class T, class Buffer_t = std::string>
    static bool json_2_obj(const Buffer_t & buf, T & obj, std::string * err_msg = nullptr)
    {
        try{
            iguana::from_json(obj, buf.data(), buf.size());
        }catch(const std::exception & e)
        {
            if(err_msg){
                *err_msg = e.what();
            }

            return false;
        }

        return true;
    }

    template<class T, template<class> class Ptr_t = std::shared_ptr, class Buffer_t = std::string>
    static Ptr_t<T> json_2_obj_ptr(const Buffer_t & buf, std::string * err_msg = nullptr)
    {
        auto ret_ptr = Ptr_t<T>(new T);
        if(!json_2_obj<T, Buffer_t>(buf, *ret_ptr)){
            ret_ptr = nullptr;
        }

        return ret_ptr;
    }

    template<class String_t = std::string, class T>
    static String_t obj_2_json(const T & obj, bool prettify = false)
    {
        String_t out_text;
        iguana::to_json(obj, out_text);

        if(prettify){
            String_t prettify_text;
            iguana::prettify(out_text, prettify_text);
            out_text = std::move(prettify_text);
        }

        return out_text;
    }


}//namespace json
}//namespace Helper

#endif // JSON_OPERATION_H
