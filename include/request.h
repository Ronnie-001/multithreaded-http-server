#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>
#include <optional>

struct Request 
{
    // Start line
    std::string method;
    std::string resource;
    std::string version; 

    // request headers
    std::map<std::string, std::string> headers;

    // message body
    std::optional<std::string> body;
};

#endif // ! REQUEST_H
