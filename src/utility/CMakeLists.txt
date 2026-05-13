cmake_minimum_required(VERSION 3.23)

# Create the object files for linking
add_library(cerberus_utility
    STATIC metrics.cpp
)

target_include_directories(cerberus_server
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
