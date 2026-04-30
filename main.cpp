#include "tcp.h"
#include <memory>

int main()
{
    auto listener = std::make_unique<TcpListener>();    

    // Start the TCP listener.
    listener->findServerAddress();
    listener->listenForConnections();

    return 0;
}
