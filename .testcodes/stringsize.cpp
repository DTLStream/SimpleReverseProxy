#include <string>
#include <iostream>

int main(){
    std::string s("shit");
    char *ptr = new char[10];
    ptr[0] = 0xbe;
    ptr[1] = 0xaf;
    std::cout<<sizeof(s)<<std::endl;
    return 0;
}