cmake_minimum_required(VERSION 3.13)
project (AFGEngine LANGUAGES CXX)

#Only the image packer depends on it for now.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )

add_subdirectory(modules)
add_subdirectory(engine)
add_subdirectory(tools)

