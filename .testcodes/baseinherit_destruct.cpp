#include <iostream>
#include <memory>
class A {
public:
    virtual ~A(){std::cout<<"AD\n";};
};

class B:public A {
public:
    ~B(){std::cout<<"BD\n";};
};

int main() {
    B* ptr = new B;
    std::shared_ptr<B> bptr(ptr);
    std::shared_ptr<A> aptr = bptr;
    // std::shared_ptr<B> bcastptr(ptr); // error
    // std::shared_ptr<B> bcastptr(bptr); // OK

    std::shared_ptr<B> bcastptr = std::dynamic_pointer_cast<B,A>(aptr);

    std::cout<<"reset aptr\n";
    aptr.reset();

    std::cout<<"reset bptr\n";
    bptr.reset();

    std::cout<<"reset bcastptr\n";
    bcastptr.reset();

    std::cout<<"reached end\n";
    
    return 0;
}