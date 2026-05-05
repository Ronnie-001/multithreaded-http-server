#include <arpa/inet.h>
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

#include "tcp.h"
#include "parser.h"

#define MY_PORT "3490"
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
{
    close(_conn_fd);
}

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

    // Register interest in the _sock_fd file descriptor
    _ev.events = EPOLLIN;
    _ev.data.fd = _sock_fd;

    int register_interest = epoll_ctl(_epoll_fd, EPOLL_CTL_ADD, _sock_fd, &_ev); 
    if (register_interest == -1) {
        perror("[ERROR] Error registering interest with file descriptor: ");
        std::cout << _sock_fd;
        exit(EXIT_FAILURE);
    } 
    
    // Create the parser here 
    auto parser = std::make_unique<cerberus::HttpParser>();

    while (_server_running) {
        // Grab the number of READY file descriptors
        int nfds = epoll_wait(_epoll_fd, &_ev, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("[ERROR] Erorr when waiting for available file descriptors.");
            exit(EXIT_FAILURE);
        }
        
        // Loop through all of the READY file descriptors.
        for (int i = 0; i < nfds; ++i) {
            int fd = _events[i].data.fd;
            
            // Connect on the socket.
            socklen_t connection_size = sizeof(_received_connection);
            _conn_fd = accept(fd, (sockaddr*)&_received_connection, &connection_size);

            if (_conn_fd == -1) {
                std::cout << "[LOGS] accept: failure extracting connection request. File descriptor: " << _conn_fd;
                continue;
            }

            std::string data = readData(_conn_fd);
            
            // New socket, use a new HTTP parser object.
            if (fd == _sock_fd) {
                auto parser = std::make_unique<cerberus::HttpParser>(_conn_fd, data);
                parser->appendData(data);
                // Transfer ownership to map.
                _parsers[fd] = std::move(parser);

            } else { // This is an existing socket, grab the associated parser and append.
                cerberus::HttpParser* parser = _parsers[fd].get();
                parser->appendData(data); 
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
        perror("[ERROR] Error creating an epoll instance."); 
        exit(EXIT_FAILURE);
    }
}

std::string cerberus::TcpListener::readData(const int _conn_fd)
{
    // Read in the incoming request data.
    char ip_address[INET6_ADDRSTRLEN];
    socklen_t request_size = sizeof(ip_address);

    inet_ntop(_received_connection.ss_family, getAddressFamily(&_received_connection), ip_address, request_size);
    std::cout << "[SERVER] IP address: " << ip_address << '\n';

    // buffer to read the data into.
    char buffer[BUFFER_SIZE];
    std::string data;

    int nread;
    while ((nread = recv(_conn_fd, buffer, BUFFER_SIZE, 0)) > 0) {
        data.append(buffer, nread);             

        std::cout << "data recieved: " << '\n';
        std::cout << "-------------DATA---------------" << '\n';
        std::cout << data << '\n';
        std::cout << "--------------------------------" << '\n';
        
        // Create a parser for each incoming request
        auto parser = std::make_unique<cerberus::HttpParser>(_conn_fd, data);
        
        cerberus::Request req = parser->constructRequest();
        std::cout << req;

        if (nread == -1) {
            continue;
        }
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
