cmake_minimum_required(VERSION 3.23)

add_library(cerberus_parser
    STATIC parser.cpp
           simdjson.cpp
)

target_include_directories(cerberus_parser
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
