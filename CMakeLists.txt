cmake_minimum_required(VERSION 3.13)
project (AFGEngine LANGUAGES CXX)

if(MINGW)
	add_compile_definitions(ALT_MINGW_THREADS=1)
endif()

add_executable(AFGEngine engine/resource.rc)

add_subdirectory(engine)
add_subdirectory(Modules)

find_package(SDL2 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(PNG REQUIRED)

target_link_libraries(AFGEngine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(AFGEngine PRIVATE glad::glad)
target_link_libraries(AFGEngine PRIVATE PNG::PNG)

install(TARGETS AFGEngine RUNTIME DESTINATION debug)

#target_link_libraries(AFGEngine uxtheme shlwapi version)