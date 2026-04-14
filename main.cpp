#include "tcp.h"

int main()
{
    TcpListener* listener = new TcpListener();   

    // Start the TCP listener.
    listener->findServerAddress();
    listener->listenForConnections();

    return 0;
}
