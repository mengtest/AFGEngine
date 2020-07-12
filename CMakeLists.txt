cmake_minimum_required(VERSION 3.13)
project (AFGEngine)

add_executable(AFGEngine resource.rc)

add_subdirectory(src)
add_subdirectory(Modules)


find_package(PkgConfig REQUIRED) 

pkg_check_modules(OGG REQUIRED IMPORTED_TARGET ogg)
pkg_check_modules(OGGZ REQUIRED IMPORTED_TARGET oggz)
pkg_check_modules(FISHSOUND REQUIRED IMPORTED_TARGET fishsound)

target_link_libraries(AFGEngine PUBLIC
	PkgConfig::OGGZ
	PkgConfig::FISHSOUND
	PkgConfig::OGG
)

find_package(glfw3 REQUIRED)
target_link_libraries(AFGEngine PUBLIC glfw)

find_package(OpenGL REQUIRED)
target_link_libraries(AFGEngine PUBLIC OpenGL::GL)

find_package(GLEW REQUIRED)
target_link_libraries(AFGEngine PUBLIC GLEW::GLEW)


find_package(PNG REQUIRED)
if(PNG_FOUND)
	target_include_directories(AFGEngine PUBLIC "${PNG_INCLUDE_DIRS}")
	target_link_libraries(AFGEngine PUBLIC "${PNG_LIBRARIES}")
	add_definitions(${PNG_DEFINITIONS})
endif()

install(TARGETS AFGEngine RUNTIME DESTINATION debug)

#target_link_libraries(AFGEngine uxtheme shlwapi version)