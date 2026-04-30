#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <unordered_map>
#include <optional>

struct Request 
{
    // Start line
    std::string method;
    std::string resourcePath;
    std::string version; 

    // request headers
    std::unordered_map<std::string, std::string> headers;

    // message body
    std::optional<std::unordered_map<std::string, std::string>> body;
};

    /*
     * Friend function of the HttpParser class, which acts as a 'pretty printer' for 
     * the constructed Request struct.
     */
    std::ostream& operator<<(std::ostream& out, const Request& request); 


#endif // ! REQUEST_H
