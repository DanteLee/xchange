#include <xchange/io/Buffer.h>

#include <iostream>
#include <string>

using xchange::io::Buffer;
using std::cout;
using std::endl;

int main(void) {
    Buffer a("this is a buffer");

    // char[], Buffer, string and rvalue are all ok
    // here will use 'move' rather than 'copy constructor'
    Buffer temp = a+" hi"+Buffer(" greate one!") + std::string(" Nicely done!");

    cout << temp << endl;

    a += " operator += is good to go";

    cout << a << endl;

    cout << "a[1]=" << a[1] << endl;

    return 0;
}
