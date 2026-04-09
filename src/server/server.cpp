#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <mutex>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include "server.h"

#define MY_PORT "3490"
#define BACKLOG 10

// Define the mutex here
std::mutex mtx;

int main()
{ 
    int status;
    // Specifies the criteria for the returned socket addresses.
    struct addrinfo hints; 
    // Collect the result here, and have a pointer to servinfo
    struct addrinfo* servinfo;
    struct addrinfo* ptr;

    int bind_res, sockfd;

    // Make sure that the struct is empty before filling it in with data
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, MY_PORT, &hints, &servinfo)) != 0) {
        std::cout << "gai error: ";
        std::cout << gai_strerror(status);
        exit(1);
    }

    // Loop through all of the found server addresses.
    for (ptr = servinfo; ptr != NULL; ptr = ptr->ai_next) {
        
        sockfd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (sockfd == -1) {
            perror("server: socket error");
            continue;
        }

        bind_res = bind(sockfd, ptr->ai_addr, ptr->ai_addrlen);

        if (bind_res == -1) {
            perror("server: bind error");
            continue;
        }
        
        break;
    } 

    freeaddrinfo(servinfo);

    if (ptr == NULL) {
        // Flush any pending output and exit
        std::cerr << "server: No avaliable connections, closing server."; 
        exit(1);
    }

    int listenfd = listen(sockfd, BACKLOG);

    if (listenfd == -1) {
        perror("server: listen error");
        exit(1);
    }   
    
    std::cout << "server: Waiting for new connections...\n"; 

    bool server_running = true;
    // Store the recieved connection
    struct sockaddr recieved_connection;

    while (server_running) {
        socklen_t connection_size = sizeof(recieved_connection);
        int new_conn_fd = accept(sockfd, &recieved_connection, &connection_size);
        
        if (new_conn_fd == -1) {
            perror("accept: error accepting a new connection");
            continue;
        }

        // Get the name of the IP address that is connecting
        char conn_ip[INET_ADDRSTRLEN];
        socklen_t ip_size = sizeof(conn_ip);
        
        struct sockaddr_in* ipv4_addr = (struct sockaddr_in*)&recieved_connection;

        inet_ntop(AF_INET, &ipv4_addr->sin_addr, conn_ip, ip_size);
        std::cout << "server: Recieved connection from: " << conn_ip << '\n'; 

        // Create a buffer to read the data into
        char buffer[1024];

        
        //  Create a child process
        //  TODO: To be removed
        pid_t pid = fork();
        if (pid == 0) {
            close(sockfd);
            
            std::string msg = "Hello from the child process!";

            send(new_conn_fd, msg.data(), msg.size(), 0);
            close(new_conn_fd);

            exit(0);
        }
        
        close(new_conn_fd);
    }
}
