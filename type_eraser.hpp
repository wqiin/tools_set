#ifndef TYPE_ERASER_HPP
#define TYPE_ERASER_HPP

#include <memory>
#include <iostream>
#include <vector>

//类型擦除。适用场景： 不同类型放同一容器，且无法/不愿共享基类。
class Drawable{
    //定义接口，也可以不定义任何借口，即可实现任意类型适配，也可以使用C++20模板的Concept限定接口类型
    struct Concept{
        virtual ~Concept() = default;
        virtual void draw() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;
    };


    template<typename T>
    struct Model : Concept{
        T obj_;

        explicit Model(T obj) : obj_(std::move(obj)){}

        void draw() const override {obj_.draw();}

        std::unique_ptr<Concept> clone() const override{
            return std::make_unique<Model<T>>(obj_);
        }
    };

private:
    std::unique_ptr<Concept> impl_;

public:
    //任意类型的构造函数  - 实例化具体的类型
    template<typename T>
    Drawable(T obj) : impl_(std::make_unique<Model<T>>(std::move(obj))){}

    Drawable(const Drawable & o) : impl_(o.impl_->clone()) {}
    Drawable(Drawable && o) noexcept = default;

    void draw() const{
        impl_->draw();
    }
};



//std::any 本质上是：一个“类型擦除（type erasure）容器”它可以：在同一个类型里，保存任意类型的对象。同时：在运行时记住真实类型。
//类型擦除。适用场景： 不同类型放同一容器，且无法/不愿共享基类。
class any_type{
    //定义接口，也可以不定义任何借口，即可实现任意类型适配，也可以使用C++20模板的Concept限定接口类型
    struct Concept{
        virtual ~Concept() = default;
        //virtual void draw() const = 0;
        virtual std::unique_ptr<Concept> clone() const = 0;//用于复制构造函数
    };


    template<typename T>
    struct Model : Concept{
        T obj_;

        explicit Model(T obj) : obj_(std::move(obj)){}

        std::unique_ptr<Concept> clone() const override{
            return std::make_unique<Model<T>>(obj_);
        }
    };

private:
    std::unique_ptr<Concept> impl_;

public:
    //任意类型的构造函数  - 实例化具体的类型
    template<typename T>
    any_type(T obj) : impl_(std::make_unique<Model<T>>(std::move(obj))){}

    any_type(const any_type & o) : impl_(o.impl_->clone()) {}
    any_type(any_type && o) noexcept = default;
};


struct Circle
{
    void draw() const {
        std::cout << "Draw functionality calling from Circle." << std::endl;
    }
};

struct Rectangle
{
    void draw() const {
        std::cout << "Draw functionality calling from Rectangle." << std::endl;
    }
};

#include <any>
void type_erase_usage()
{
    //直接在 optional 内部构造， 避免一次 move。
    std::optional<std::string> op(std::in_place, 0, 'A');

    std::vector<Drawable> shapes{
        Circle{},
        Rectangle{}
    };

    shapes.emplace_back(Circle{});
    shapes.emplace_back(Rectangle{});
    shapes.emplace_back(Circle{});

    for(const auto & shape : shapes)
    {
        shape.draw();
    }

    std::vector<any_type> anys{
        12, 23, 23.0, "std::string"
    };

    // for(const auto & item : anys){
    //     std::cout << item << std::endl;
    // }

    std::any ay;

    int nX = 23;
    //using aaaa = ;typeid(int)

    //typeid(int) -> 得到的是type_info对象;   不能得到原始的类型，因为类型不是运行时的值，类型在编译的时候就确定了

    //typeid();//为运行时的类型识别, 可以推导有继承关系的类，
    //decltype//为编译期的类型推导

}


#endif // TYPE_ERASER_HPP
