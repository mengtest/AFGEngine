cmake_minimum_required(VERSION 3.13)
project (AFGEngine LANGUAGES CXX)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )

find_package(SDL2 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory(Modules)
add_subdirectory(engine)
add_subdirectory(tools)

