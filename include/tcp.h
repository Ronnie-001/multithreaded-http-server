#ifndef TCP_H
#define TCP_H

#include <netdb.h>

class TcpListener {

private:
    int _status;

    // Member variables to hold the socket file descriptors.
    int _sock_fd;
    int _bind_fd;
    int _listen_fd;
    int _conn_fd;

    struct addrinfo _hints;
    struct addrinfo* _servinfo;
    struct addrinfo* _ptr;

    bool _server_running;
    struct sockaddr_storage _received_connection; 

public:
    // Constructor
    TcpListener();
    
    // Destructor
    ~TcpListener();
    
    /*
     * Gets a list of server addresses to connect on using getaddrinfo() and 
     * addrinfo _hints for specifying criteria to look for when scanning through.
     */
    void findServerAddress();
    
     // Used for listening for incoming connections.
    void listenForConnections();
    
    /* Used to get the address family of the recieved connection; determines if it's 
     * IPv4 or IPv6.
     * Returns the internet socket address of the received connection.
    */
    void* getAddressFamily(const sockaddr_storage* recieved_connection);
}; 

#endif // ! TCP_H
