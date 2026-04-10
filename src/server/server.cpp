#include <cstdio>
#include <cstdlib>
#include <netdb.h>
#include <__stddef_null.h>
#include <sys/socket.h>
#include <iostream>

#define MY_PORT "3490"
#define BACKLOG 10

class Server {

private:
    int _status;

    // Member variables to hold the socket file descriptors.
    int _sock_fd;
    int _bind_fd;
    int _listen_fd;
    int _accept_fd;

    struct addrinfo _hints;
    struct addrinfo* _servinfo;
    struct addrinfo* _ptr;

    bool _server_running;
    struct sockaddr _received_connection; 

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
    ~Server() {}
    
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
        
        // Try each found server address until successful bind()
        for (_ptr = _servinfo; _ptr != NULL; _ptr = _ptr->ai_next) {

            _sock_fd = socket(_ptr->ai_family, _ptr->ai_socktype, _ptr->ai_protocol);

            if (_sock_fd != -1) {
                std::cerr << "[LOGS] Failed to create endpoint using socket() on server address: " << _ptr->ai_addr << "Trying next..."<< "\n"; 
                continue;     
            }

            _bind_fd = bind(_sock_fd, _ptr->ai_addr, _ptr->ai_addrlen);

            if (_bind_fd == -1) {
               std::cerr << "[LOGS] Failed to bind on server address: " << _ptr->ai_addr << "Trying next..."<< "\n"; 
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
    
    /*
     * Function used for listening for incoming connections
     */
    void listenForConnections()
    {
        // Set the queue size through BACKLOG
        _listen_fd = listen(_sock_fd, BACKLOG);
        
        if (_listen_fd == -1) {
            std::cerr << "[ERROR] Error when listening for connection on socket with file descriptor: " << _sock_fd;
            exit(EXIT_FAILURE);
        }
        
        std::cout << "[LOGS] Server: listening for incoming connections..." << "\n";

        _server_running = true;

        while (_server_running) {
            socklen_t connection_size = sizeof(_received_connection);

        }        
    }
};
