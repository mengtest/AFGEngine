cmake_minimum_required(VERSION 3.13)
project (AFGEngine LANGUAGES CXX)

find_package(SDL2 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(PNG REQUIRED)

add_subdirectory(Modules)
add_subdirectory(engine)
add_subdirectory(tools)

