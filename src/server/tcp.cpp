#include <arpa/inet.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>
#include <memory>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <sys/epoll.h>
#include <fcntl.h>

#include "tcp.h"
#include "parser.h"
#include "metrics.h"

#define MY_PORT "8000"
#define BACKLOG 10
#define BUFFER_SIZE 1024

cerberus::TcpListener::TcpListener() : _server_running(false) 
{
    // Look for IPv4 or IPv6.
    _hints.ai_family = PF_UNSPEC;
    // Look at server addresses that use TCP.
    _hints.ai_socktype = SOCK_STREAM;
    _hints.ai_flags = AI_PASSIVE;
   
    // Get the server addresses that match the hints.
   _status = getaddrinfo(NULL, MY_PORT, &_hints, &_servinfo);
}

cerberus::TcpListener::~TcpListener() 
{}

void cerberus::TcpListener::findServerAddress()
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
        std::cerr << "[ERROR] TcpListener: No avaliable connections, closing server.";
        exit(EXIT_FAILURE);
    }
}

void cerberus::TcpListener::listenForConnections() 
{
    // Set the queue size through BACKLOG.
    _listen_fd = listen(_sock_fd, BACKLOG);
    
    if (_listen_fd == -1) {
        std::cerr << "[ERROR] listen: Error when listening for connection on socket with file descriptor: " << _sock_fd;
        exit(EXIT_FAILURE);
    }
    
    std::cout << "[SERVER]: listening for incoming connections..." << "\n";

    _server_running = true;
    createEpollInstance();
    
    // Set the socket to be non-blocking
    int set_fd = setNonBlocking(_sock_fd);
    if (set_fd == -1) {
        std::cerr << "[ERROR] Error when trying to set the socket non non-blocking";
        exit(EXIT_FAILURE);
    }

    // Register interest in the _sock_fd file descriptor
    _ev.events = EPOLLIN | EPOLLET;
    _ev.data.fd = _sock_fd;

    int register_interest = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock_fd, &_ev); 
    if (register_interest == -1) {
        std::cerr << "[ERROR] Error registering interest with file descriptor: " << _sock_fd;
        exit(EXIT_FAILURE);
    } 

    while (_server_running) {
        // Grab the number of READY file descriptors
        int nfds = epoll_wait(_epoll_fd, _events, MAX_EVENTS, -1);
        if (nfds == -1) {
            std::cerr << "[ERROR] Erorr when waiting for available file descriptors.";
            exit(EXIT_FAILURE);
        }
        
        // Loop through all of the READY file descriptors.
        for (int i = 0; i < nfds; ++i) {
            int fd = _events[i].data.fd;
            // New socket, use a new HTTP parser object.
           if (fd == _sock_fd) {
                // TODO: loop through and accept all connections on this file descriptor.
                while (true) {
                    socklen_t connection_size = sizeof(_received_connection);

                    int _conn_fd = accept(fd, (sockaddr*)&_received_connection, &connection_size);
                    if (_conn_fd == -1) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        }
                        std::cout << "[LOGS] accept: failure extracting connection request. File descriptor: " << _conn_fd;
                        continue;
                    }
                    
                    int set_fd = setNonBlocking(_conn_fd);
                    if (set_fd == -1) {
                        std::cerr << "[ERROR] Error when trying to set the connection fd to non non-blocking";
                        exit(EXIT_FAILURE);
                    }

                    std::string data = readData(_conn_fd);

                    // Edge triggered.
                    _ev.events = EPOLLIN | EPOLLET;
                    _ev.data.fd = _conn_fd;
                    
                    int register_interest = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _conn_fd, &_ev);
                    if (register_interest == -1) {
                        std::cerr << "[ERROR] Error registering interest with file descriptor: " << _sock_fd;
                        exit(EXIT_FAILURE);
                    } 

                    auto parser = std::make_unique<cerberus::HttpParser>(_conn_fd, data);
                    parser->appendData(data);

                    if (!parser->isRequestComplete()) {
                        // Transfer ownership to map.
                        _parsers[fd] = std::move(parser);
                        continue;
                    } else {
                        parseHttpRequest(parser.get());
                        cerberus::Request req = parser->constructRequest();
                        std::cout << req << '\n';
                    }
                }
           } else { // This is an existing socket, grab the associated parser through the file descriptor and append.
                cerberus::HttpParser* parser = _parsers[fd].get();

                std::string data = readData(fd);
                parser->appendData(data); 

                if (parser->isRequestComplete()) {
                    cerberus::Request req = parser->constructRequest();
                    std::cout << req;
                    
                    // TODO: Pass request to queue to be processed by different threads.
                }
           }
        }
    } 

    _server_running = false;
}

void* cerberus::TcpListener::getAddressFamily(const sockaddr_storage* recieved_connection)
{
    // Check for IPv4.
    if (recieved_connection->ss_family == AF_INET) {
        return &(((struct sockaddr_in*)recieved_connection))->sin_addr;
    }
    
    return &(((struct sockaddr_in6*)recieved_connection))->sin6_addr;
}

void cerberus::TcpListener::createEpollInstance() 
{
    _epoll_fd = epoll_create1(0);
    if (_epoll_fd == -1) {
        std::cerr << "[ERROR] Error creating an epoll instance."; 
        exit(EXIT_FAILURE);
    }
}

int cerberus::TcpListener::setNonBlocking(const int fd) 
{
    int flags = fcntl(fd, F_GETFL, 0); 
    if (flags == -1) {
        std::cerr << "[ERROR] Error getting the file access mode and status flags.";
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "[ERROR] Error setting the socket file descriptor to be non-blocking";
        return -1;
    }

    return 0;
}

std::string cerberus::TcpListener::readData(const int _conn_fd)
{
    // Read in the incoming request data.
    char ip_address[INET6_ADDRSTRLEN];
    socklen_t request_size = sizeof(ip_address);

    inet_ntop(_received_connection.ss_family, getAddressFamily(&_received_connection), ip_address, request_size);
    // std::cout << "[SERVER] IP address: " << ip_address << '\n';

    // buffer to read the data into.
    char buffer[BUFFER_SIZE];
    std::string data;

    int nread;
    while ((nread = recv(_conn_fd, buffer, BUFFER_SIZE, 0))) {
        if (nread == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            break;
        }

        // Check if the client disconnects.
        if (nread == 0) {
            break;
        }

        data.append(buffer, nread);             

        std::cout << "data recieved: " << '\n';
        std::cout << "-------------DATA---------------" << '\n';
        std::cout << data;
        std::cout << "--------------------------------" << '\n';
    }

    return data;
}

void cerberus::TcpListener::parseHttpRequest(cerberus::HttpParser* parser) 
{
    parser->extractStartLine();
    parser->parseStartLine();

    parser->extractHeaders();
    parser->parseHeaders();
}

