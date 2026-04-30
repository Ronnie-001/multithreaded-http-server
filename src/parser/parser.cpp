#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "parser.h"
#include "request.h"
#include "simdjson.h"

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
    
    _extracted_start_line = start_line;
}

void HttpParser::parseStartLine()
{
    std::stringstream ss(_extracted_start_line);
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

void HttpParser::extractHeaders()
{
    std::string::size_type start = _request.find("\r\n");
    std::string::size_type end = _request.find("\r\n\r\n");

    // Take everything between the start line and \r\n\r\n. 
    _extracted_headers = _request.substr(start + 2, end - (start + 2));
}

void HttpParser::parseHeaders() 
{
    std::size_t parse_start = 0;
    std::size_t parse_end = _extracted_headers.find("\r\n");
    
    // Iterate through manually until no CLRF
    while (parse_end != std::string::npos) {
        std::string header = _extracted_headers.substr(parse_start, parse_end - parse_start);  

         // Split the header based on ": ", then add to std::map _headers
        std::stringstream ss(header);
        std::vector<std::string> v;
        std::string store;
        
        while (std::getline(ss, store, ':')) {
            v.push_back(store);
        }
        
        if (v.size() == 0) {
            parse_start = parse_end + 2;
            parse_end = _extracted_headers.find("\r\n", parse_start);
            continue; 
        }

        std::string value;
        for (int i = 1; i < v.size(); i++) {
            value.append(v[i]);
        }
        
        // Remove the leading whitespace if found.
        value = value.find(' ') != std::string::npos ? value.substr(1) : value;

        std::cout << "------------------------" << '\n';

        std::cout << "[" << v[0] << "]" << '\n';
        std::cout << value << '\n';

        // Add to map
        _headers.insert({v[0], value});

        parse_start = parse_end + 2;
        parse_end = _extracted_headers.find("\r\n", parse_start);
    }

    if (_headers.contains("Content-Length")) {
        extractMessageBody();
        parseMessageBody();
    }
} 

void HttpParser::extractMessageBody() 
{
    std::string::size_type start = _request.find("\r\n\r\n") + 4;
    std::string::size_type end = _request.size() - start;
    
    _extracted_message_body = _request.substr(start, end);
}

void HttpParser::parseMessageBody() 
{    
    // Create a thread local parser
    simdjson::ondemand::document message_body = simdjson::ondemand::parser::get_parser().iterate(_extracted_message_body);
    simdjson::ondemand::object object = message_body.get_object();

    for (auto field : object) {
        // Get the key and the value
        std::string_view key = std::string_view(field->unescaped_key());
        std::string_view value = std::string_view(field->value().raw_json());

        // Remove quotes if present
        value = value.find("\"") != std::string::npos ? value.substr(1, value.size() - 2) : value;

        // Add to the map
        _message_body.insert({std::string(key), std::string(value)});
    } 
}

std::ostream& operator<<(std::ostream& out, const Request& request) 
{
    out << "------------START LINE--------" << '\n';
    out << "METHOD: " << request.method << '\n';
    out << "RESOURCE PATH: " << request.resourcePath << '\n';
    out << "VERSION: " << request.version << '\n';
    
    // Create a lambda eor printing out the headers
    auto print_key_values = [](const auto& key, const auto& value, std::ostream& out) {
        out << "HEADER: " << key << '\n';
        out << "VALUE: " << value << '\n';
    };

    out << "----------HTTP HEADERS---------" << '\n';
    for (const auto& [header, value] : request.headers) {
        print_key_values(header, value, out); 
        out << "---------------------------" << '\n';
    } 
    
    // Check if there is a message body present
    if (request.body.has_value()) {
        out << "--------------MESSAGE BODY------------" << '\n';
        for (const auto& [key, value] : *request.body) {
            print_key_values(key, value, out);
            out << "---------------------------" << '\n';
        }
    }

    return out;
}

Request HttpParser::constructRequest() 
{
    Request req;

    // init member variables 
    req.method = _method;
    req.resourcePath = _resource_path;
    req.version = _version;

    req.headers = _headers;
    req.body = _message_body;

    return req;
}
