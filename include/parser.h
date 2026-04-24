#ifndef PARSER_H
#define PARSER_H

#include <map>
#include <string>
#include "request.h"

#define BUFFER_SIZE 1024

// Used for parsing incoming HTTP requests accepter from the TCP listener.
class HttpParser 
{
private:
    // Used to check if the entire HTTP request has been recieved.
    bool _complete;
    // The fd of the socket that we are connected to.
    int _conn_fd;
    std::string _request;

    // the start line
    std::string _start_line;
    std::string _method;
    std::string _resource_path;
    std::string _version;

    // Headers 
    std::map<std::string, std::string> _headers;

    bool _has_message_body;

    // The final request to be constructed.
    Request _parsed_request;
public:
    // Constructor 
    HttpParser(int fd, const std::string request);

    // Destructor
    ~HttpParser();
    
    bool isRequestComplete() const; 

    std::string getMethod() const;
    std::string getResourcePath() const;
    std::string getVersion() const;

    /*
     * Used for appending data from the recv() system call
     * and appending it to _request.
    */
    void appendData(const char* buffer, int bytes);
    
    /* Used for extracting the start line from the HTTP request, which
     * contains the method, resource path and HTTP version,
     */
    void extractStartLine();
    
    // Used for parsing the start line, reteriving the method, resource path and version.
    void parseStartLine(); 
    
    // Used for getting the headers of the HTTP request
    void parseHeaders();
    
    // Function used to create the pasersed HTTP request.
    Request constructRequest();
};

#endif // ! PARSER_H
