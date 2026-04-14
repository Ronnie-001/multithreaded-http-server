#ifndef PARSER_H
#define PARSER_H

#include <string>

// Used for parsing incoming HTTP requests accepter from the TCP listener.
class HttpParser 
{
private:
    // Used to check if the entire HTTP request has been recieved.
    bool _complete;
    std::string _request;

public:
    // Constructor 
    HttpParser();

    // Destructor
    ~HttpParser();
    
    // Returns the state of _request.
    bool isRequestComplete() const;
    
    /*
     * Function used for appending data from the recv() system call
     * and appending it to _request.
    */
    void appendData();
    
    // Used for extracting the method used, i.e. GET, POST, PUT, etc.
    std::string extractMethod(); 
    
    // Used for extracting the path to the resource.
    std::string extractResourcePath();
    
    // Used for extracting the HTTP version being used.
    std::string extractVersion();

     
};

#endif // ! PARSER_H
