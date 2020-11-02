cmake_minimum_required(VERSION 3.13)
project (AFGEngine)

if(MINGW)
	add_compile_definitions(ALT_MINGW_THREADS=1)
endif()

add_executable(AFGEngine resource.rc)

add_subdirectory(src)
add_subdirectory(Modules)

find_package(glfw3 CONFIG REQUIRED)
target_link_libraries(AFGEngine PRIVATE glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(AFGEngine PRIVATE OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(AFGEngine PRIVATE GLEW::GLEW)


find_package(PNG REQUIRED)
if(PNG_FOUND)
	target_include_directories(AFGEngine PRIVATE "${PNG_INCLUDE_DIRS}")
	target_link_libraries(AFGEngine PRIVATE "${PNG_LIBRARIES}")
	add_definitions(${PNG_DEFINITIONS})
endif()

install(TARGETS AFGEngine RUNTIME DESTINATION debug)

#target_link_libraries(AFGEngine uxtheme shlwapi version)