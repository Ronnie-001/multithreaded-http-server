#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <__stddef_null.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <iostream>
#include <unistd.h>

#define MY_PORT "3490"
#define BACKLOG 10

class Server {

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
    Server() : _server_running(false)
    {
        // Look for IPv4 or IPv6.
        _hints.ai_family = PF_UNSPEC;
        // Look server addresses that use TCP.
        _hints.ai_socktype = SOCK_STREAM;
        _hints.ai_flags = AI_PASSIVE;
        
        // Get the server addresses that match the hints.
        _status = getaddrinfo(NULL, MY_PORT, &_hints, &_servinfo);
    }
    
    // Destructor
    ~Server() {
        // Close the connection.
        close(_conn_fd);
    }
    
    /*
     * Gets a list of server addresses to connect on using getaddrinfo() and 
     * addrinfo _hints for specifying criteria to look for when scanning through.
     */
    void findServerAddress() 
    {
        if (_status != 0) {
            fprintf(stderr, "[ERROR] getaddrinfo: %s\n", gai_strerror(_status));
            exit(EXIT_FAILURE);
        }
        
        // Try each found server address until successful bind().
        for (_ptr = _servinfo; _ptr != NULL; _ptr = _ptr->ai_next) {

            _sock_fd = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol);

            if (_sock_fd == -1) {
                std::cerr << "[LOGS] socket: failed to create endpoint using socket() on server address: " << _ptr->ai_addr << "Trying next..."<< "\n"; 
                continue;     
            }

            _bind_fd = bind(_sock_fd, _ptr->ai_addr, _ptr->ai_addrlen);

            if (_bind_fd == -1) {
               std::cerr << "[LOGS] bind: failed to bind on server address: " << _ptr->ai_addr << "Trying next..."<< "\n"; 
               continue;     
            }
            
            // Exit loop once you have found a server address to connect on.
            break;
        }

        freeaddrinfo(_servinfo);

        // No connections found.
        if (_ptr == NULL) {
            // Flush the pending output and exit 
            std::cerr << "[ERROR] Server: No avaliable connections, closing server.";
            exit(EXIT_FAILURE);
        }
    }
    
     // Used for listening for incoming connections.
    void listenForConnections()
    {
        // Set the queue size through BACKLOG.
        _listen_fd = listen(_sock_fd, BACKLOG);
        
        if (_listen_fd == -1) {
            std::cerr << "[ERROR] listen: Error when listening for connection on socket with file descriptor: " << _sock_fd;
            exit(EXIT_FAILURE);
        }
        
        std::cout << "[SERVER]: listening for incoming connections..." << "\n";

        _server_running = true;

        while (_server_running) {
            socklen_t connection_size = sizeof(_received_connection);
            _conn_fd = accept(_sock_fd, (sockaddr*)&_received_connection, &connection_size);
            
            if (_conn_fd == -1) {
                std::cout << "[LOGS] accept: failure extracting connection request. File descriptor: " << _conn_fd;
                continue;
            }
            
            // Read in the incoming request data.
            char data[INET6_ADDRSTRLEN];
            socklen_t request_size = sizeof(data);

            inet_ntop(_received_connection.ss_family, getAddressFamily(_received_connection), data, request_size);
            std::cout << "[SERVER] Data recieved: " << data << '\n';

            // buffer to read the data into.
            char buffer[1024];
        }        
    }
    
    /* Used to get the address family of the recieved connection; determines if it's 
     * IPv4 or IPv6.
     * Returns the internet socket address of the received connection.
    */
    void* getAddressFamily(const sockaddr_storage& recieved_connection) {
        
        // Check for IPv4.
        if (recieved_connection.ss_family == AF_INET) {
            struct sockaddr_in* s = (struct sockaddr_in*)&recieved_connection;
            return &(s->sin_addr);
        }
        
        // IPv6.
        struct sockaddr_in6* s = (struct sockaddr_in6*)&recieved_connection;
        return &(s->sin6_addr);
    }
}; 
