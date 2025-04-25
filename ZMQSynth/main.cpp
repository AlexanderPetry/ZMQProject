#include <iostream>
#include <zmq.hpp>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

int main() {
    try {

        printf("test");

    } catch (zmq::error_t &ex) {
        std::cerr << "Caught an exception : " << ex.what() << std::endl;
    }

    return 0;
}
