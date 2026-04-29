#ifndef XML_OPERATION_H
#define XML_OPERATION_H

#include "iguana/xml_reader.hpp"
#include "iguana/xml_writer.hpp"
#include "iguana/prettify.hpp"

namespace Helper {
namespace xml {
    template<class T, class Buffer_t = std::string>
    static bool xml_to_obj(const Buffer_t & buf, T & obj, std::string * err_msg = nullptr)
    {
        try{
            iguana::from_xml(obj, buf);
        }catch(const std::exception & e){
            if(err_msg){
                *err_msg = e.what();
            }

            return false;
        }

        return true;
    }

    template<class T, template<class> class Ptr_t = std::shared_ptr, class Buffer_t = std::string>
    static Ptr_t<T> xml_to_obj_ptr(const Buffer_t & buf, std::string * err_msg = nullptr)
    {
        auto ret_ptr = Ptr_t<T>(new T);
        if(!xml_2_obj<T, Buffer_t>(buf, *ret_ptr)){
            ret_ptr = nullptr;
        }

        return ret_ptr;
    }

    template<class String_t = std::string, class T>
    static String_t obj_to_xml(const T & obj)
    {
        String_t out_text;
        iguana::to_xml(obj, out_text);

        return out_text;
    }
}
}


#endif // XML_OPERATION_H
