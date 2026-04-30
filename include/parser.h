#ifndef PARSER_H
#define PARSER_H

#include <unordered_map>
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

    std::string _extracted_start_line;
    std::string _method;
    std::string _resource_path;
    std::string _version;
   
    // Headers
    std::string _extracted_headers;
    std::unordered_map<std::string, std::string> _headers;

    // Message body
    std::string _extracted_message_body;
    std::unordered_map<std::string, std::string> _message_body;

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
   
    // Used for extracting the headers with the CLRF.
    void extractHeaders();
    // Used for getting the headers of the HTTP request
    void parseHeaders();
        
    // Used for getting the JSON message body from the HTTP request.
    void extractMessageBody();
    // Used for parsing each individual field from the extracted message body 
    void parseMessageBody();

    // Function used to create the pasersed HTTP request.
    Request constructRequest();
};

#endif // ! PARSER_H
