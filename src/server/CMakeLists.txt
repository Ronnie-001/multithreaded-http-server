cmake_minimum_required(VERSION 3.23)
project(cerberus_server)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Create the object files for linking
add_library(cerberus_server
    STATIC server.cpp
)
