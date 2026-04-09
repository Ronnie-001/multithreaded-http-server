# Name of project and CMake version
cmake_minimum_required(VERSION 3.23)
project(cerberus)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# TODO: Set the subdirectories for the project (parser, response, router, etc.)
add_subdirectory(/src/server)

add_executable(Server main.cpp)

# TODO: Link the executable with the libraries
target_link_libraries(Server 
    PRIVATE cerberus_server
)
