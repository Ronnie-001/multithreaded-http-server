#include <sstream>
#include <string>
#include <vector>

#include "parser.h"

HttpParser::HttpParser(int fd, const std::string request) : _complete(false), _conn_fd(fd), _request(request)
{}

HttpParser::~HttpParser() { /*Nothing here!!!*/ } 

bool HttpParser::isRequestComplete() const { return _complete; }

std::string HttpParser::getMethod() const { return _method; }
std::string HttpParser::getResourcePath() const { return _resource_path; }
std::string HttpParser::getVersion() const { return _version; }


void HttpParser::appendData(const char* buffer, int bytes)
{
    _request.append(buffer, bytes);
}

void HttpParser:: extractStartLine()
{
    // Look for first instance of CRLF
    std::string::size_type first_clrf = _request.find("\r\n");
    std::string start_line = _request.substr(0, first_clrf);  
    
    _start_line = start_line;
}

void HttpParser::parseStartLine()
{
    std::stringstream ss(_start_line);
    // Use a vector to store each part of the start line.
    std::vector<std::string> v;
    std::string store;
    
    while (std::getline(ss, store, ' ')) {
        v.push_back(store);
    }
        
    // Set the member variables pertaining to the start line
    _method = v[0];
    _resource_path = v[1];
    _version = v[2];
}

void HttpParser::parseHeaders() 
{
    std::string::size_type start = _request.find("\r\n");
    std::string::size_type end = _request.find("\r\n\r\n");

    // Take everything between the start line and the space.
    std::string headers = _request.substr(start, end);
    
    std::size_t parse_start = 0;
    std::size_t parse_end = headers.find("\r\n");
    
    // Iterate through manually until no CLRF
    while (parse_end != std::string::npos) {
        std::string header = headers.substr(parse_start, parse_end - parse_start);  

         // Split the header based on ": ", then add to std::map _headers
        std::stringstream ss(header);
        std::vector<std::string> v;
        std::string store;
        
        while (std::getline(ss, store, ':')) {
            v.push_back(store);
        }

        // If v has no elements, then move on
        if (v.size() == 0) {
            parse_start = parse_end + 2;
            parse_end = headers.find("\r\n", parse_start);
            continue; 
        }

        std::string value;
        for (int i = 1; i < v.size(); i++) {
            value.append(v[i]);
        }
        
        // Remove the leading whitespace if found.
        value = value.find(' ') != std::string::npos ? value.substr(1) : value;

         // Add to map
         _headers.insert({v[0], value});

        parse_start = parse_end + 2;
        parse_end = headers.find("\r\n", parse_start);
    }
} 
