#include <server.h>

int main()
{
    Server* server = new Server();
    
    // Start the TCP listener.
    server->findServerAddress();
    server->listenForConnections();

    return 0;
}
