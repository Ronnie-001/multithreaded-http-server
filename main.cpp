#include "tcp.h"
#include <memory>

int main()
{
    auto listener = std::make_unique<cerberus::TcpListener>();    

    // Start the TCP listener.
    listener->findServerAddress();
    listener->listenForConnections();

    return 0;
}
