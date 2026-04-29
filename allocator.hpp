#ifndef ALLOCATOR_HPP
#define ALLOCATOR_HPP


#include <cstddef>//std::ptrdiff_t
#include <new>//std::set_new_handler
#include <iostream>

namespace JJ
{
    //实现自定义的内存分配器
    template<class T>
    inline T * _allocate(std::ptrdiff_t size, T *)
    {
        std::set_new_handler(0);
        T* tmp = (T*) (::operator new((size_t)(size * sizeof(T))));

        if(nullptr == tmp){
            std::cerr << "Out of Memory" << std::endl;
            std::exit(1);
        }

        return tmp;
    }


    template<class T>
    inline void _deallocate(T * buf)
    {
        ::operator delete(buf);
    }

    template<class T1, class T2>
    inline void _construct(T1 * p, const T2 & value)
    {
        new (p) T1(value);//placement new
    }


    template<class T>
    inline void _destory(T * p)
    {
        p->~T();//calling its deconstruction functionality mannually
    }


    template<class T>
    class allocator{
    public:
        using value_type = T;
        using pointer = T*;
        using const_pointer = const T *;
        using reference = T&;
        using const_reference = const T&;
        using size_type = size_t;
        using difference_type = ptrdiff_t;

        //rebind allocator of type U
        template<class U>
        struct rebind{
            using other = allocator<U>;
        };

        pointer allocate(size_type n, const void * hint = nullptr){
            return _allocate(difference_type(n), pointer(hint));
        }


        void deallocate(pointer p, size_type n)
        {
            _deallocate(p);
        }

        void construct(pointer p, const T & value)
        {
            _construct(p, value);
        }

        void destory(pointer p){
            _destory(p);
        }

        pointer addesss(reference x)
        {
            return pointer(&x);
        }

        const_pointer const_address(const_reference x)
        {
            return const_pointer(&x);
        }

        size_type max_size() const
        {
            return size_type(UINT_MAX / sizeof(T));
        }

    };

}


#endif // ALLOCATOR_HPP
