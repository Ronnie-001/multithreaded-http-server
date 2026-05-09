#ifndef TCP_H
#define TCP_H

#include "parser.h"
#include <memory>
#include <netdb.h>
#include <sys/epoll.h>
#include <unordered_map>

#define MAX_EVENTS 15

namespace cerberus 
{
    class TcpListener 
    {

        private:
            int _status;

            // Member variables to hold the socket file descriptors.
            int _sock_fd;
            int _bind_fd;
            int _listen_fd;

            struct addrinfo _hints;
            struct addrinfo* _servinfo;
            struct addrinfo* _ptr;

            bool _server_running;
            struct sockaddr_storage _received_connection; 

            // epoll
            int _epoll_fd;
            struct epoll_event _ev; // the current events
            struct epoll_event _events[MAX_EVENTS]; // holds all events
                                                    //
            /*
             * Map used for ownsership; it stores each HTTP parser with each TCP connection.
             * This way, when handling things such as packet switching, we can append
             * incoming data with thier correct parser.
             */
            std::unordered_map<int, std::unique_ptr<cerberus::HttpParser>> _parsers;

        public:
            // Constructor
            TcpListener();

            // Destructor
            ~TcpListener();
            
            TcpListener(const TcpListener& listener) = delete;
            TcpListener(const TcpListener&& listener) = delete;

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
            
            // Used to create a new epoll instace in kernel space.
            void createEpollInstance();
            
            // Used to set a file descriptor to be non blocking.
            int setNonBlocking(const int fd);
            
            // Function used to read in data from the recv system call.
            std::string readData(const int _conn_fd);
            
            // Used to parse and extract the start line, and HTTP headers, and potential message body.
            void parseHttpRequest(cerberus::HttpParser* parser);
    };
}

#endif // ! TCP_H
