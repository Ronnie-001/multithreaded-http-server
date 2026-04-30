# Name of project and CMake version
cmake_minimum_required(VERSION 3.23)
project(cerberus)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Enable compile commands to work with clangd LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include the subdirectories
add_subdirectory(src/server)
add_subdirectory(src/parser)

add_executable(Server main.cpp)

target_link_libraries(Server 
    PRIVATE cerberus_server 
            cerberus_parser
)
