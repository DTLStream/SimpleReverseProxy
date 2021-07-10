#include <iostream>

struct A{
    A(){std::cout<<"A created\n";};
    ~A(){std::cout<<"A destructed\n";};
};
struct Base{
    Base(){std::cout<<"Base created\n";};
    virtual ~Base() = default;
};
struct Derived:public Base{
    Derived(){std::cout<<"Derived created\n";};
    virtual ~Derived() = default;
    A a;
};

int main(){
    Derived *d = new Derived();
    Base *b = d;
    delete b;
    std::cout<<"reached end\n";
    return 0;
}