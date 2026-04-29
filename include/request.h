#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <string_view>
#include <unordered_map>

struct Request 
{
    // Start line
    std::string method;
    std::string resourcePath;
    std::string version; 

    // request headers
    std::unordered_map<std::string_view, std::string_view> headers;

    // message body
    std::unordered_map<std::string_view, std::string_view> body;
};

#endif // ! REQUEST_H
