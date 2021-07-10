#include <thread>
#include <iostream>
#include <unistd.h>

class A {
    public:
    char *ptr;
    A():ptr(nullptr){std::cout<<"AC\n";ptr=new char[2];ptr[0]='Q';};
    A(A&& a):ptr(nullptr){std::cout<<"ACM\n";char *p=a.ptr;a.ptr=ptr;ptr=p;};
    A(const A&):ptr(nullptr){std::cout<<"ACC\n";ptr=new char[2];ptr[0]='Q';std::cout<<ptr[0]<<"\n";};
    void outpointer() const{std::cout<<ptr[0]<<"\n";std::cout<<std::hex<<this<<"\n";};
    ~A(){std::cout<<"AD\n";delete [] ptr;};
};
void thread_call(const A &a) {
    std::cout<<"registering thread_call\n";
    std::thread t(
        [a](){ // &a cause segfault
            std::cout<<"begin thread_call\n";sleep(2);std::cout<<"thread_call\n";a.outpointer();
        }
    );
    ////// following code still causes segfault/////////////////////////
    // const A &aa = A(a);
    // std::thread t(
    //     [&aa](){ // &a cause segfault
    //         std::cout<<"begin thread_call\n";sleep(2);std::cout<<"thread_call\n";aa.outpointer();
    //     }
    // );
    ////////////////////////////////////////////////////////////////////
    t.detach();
};
void thread() {
    A a;
    std::cout<<"thread\n";
    a.outpointer();
    
    thread_call(A());

    std::cout<<"thread end\n";
    a.outpointer();

    std::cout<<"thread truly end\n";
    A aa((A()));
    a.outpointer();
};

int main(){
    std::thread t(thread);
    t.join();
    sleep(3);
    return 0;
}