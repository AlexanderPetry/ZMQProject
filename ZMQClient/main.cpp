//includes
#include <Windows.h>
#include <iostream>
#include <zmq.hpp>
#include <string>

#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

bool local = 0;

std::string adress1 = "tcp://benternet.pxl-ea-ict.be:24041";
std::string adress2 = "tcp://benternet.pxl-ea-ict.be:24042";
std::string ladress1 = "tcp://127.0.0.1:24041";
std::string ladress2 = "tcp://127.0.0.1:24042";


int main() {
    try {

        zmq::context_t context(1);

        zmq::socket_t publisher(context,  local ? ZMQ_PUB : ZMQ_PUSH);
        zmq::socket_t receiver(context, ZMQ_SUB);

        if (local)
        {
            publisher.connect(ladress2); // connect to service's SUB
            receiver.connect(ladress1);  // connect to service's PUB
            std::cout << "Connected locally to " << ladress1 << " and " << ladress2 << std::endl;
        } else
        {
            publisher.connect(adress1); // connect to service's SUB
            receiver.connect(adress2);  // connect to service's PUB
            std::cout << "Connected to Benternet at " << adress1 << " and " << adress2 << std::endl;
        }

        receiver.setsockopt(ZMQ_SUBSCRIBE, "readnum!>", 9);
        receiver.setsockopt(ZMQ_RCVTIMEO, 3000);
        std::cout << "Subscribed to 'readnum!>'" << std::endl;

        sleep(1000); // give time for connections to fully register

        std::string request = "note.play?>10 127 1";
        zmq::message_t msg(request.begin(), request.end());
        publisher.send(msg);
        std::cout << "Sent request: " << request << std::endl;
        sleep(100);

        zmq::message_t reply;
        if (receiver.recv(&reply)) {
            std::string response(static_cast<char*>(reply.data()), reply.size());
            std::cout << "Received response: " << response << std::endl;
        } else {
            std::cout << "No response received (timeout)." << std::endl;
        }

    } catch (zmq::error_t &ex) {
        std::cerr << "Caught an exception : " << ex.what() << std::endl;
    }

    return 0;
}
