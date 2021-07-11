cmake_minimum_required(VERSION 3.13...3.19)
project (AFGEngine VERSION 0.2 LANGUAGES C CXX)

set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin CACHE STRING "" )

set (CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake" )

set( AFGE_BUILD_TOOLS ON CACHE BOOL "Build tools to edit game data")
set( AFGE_USE_SUBMODULES ON CACHE BOOL "Use git submodules. Turn off if you want to use a package manager instead.")

include(glad)
include(sol2)
include(includes)
if(AFGE_USE_SUBMODULES)
	set(SKIP_INSTALL_ALL ON)
	add_subdirectory(submodules/zlib)
	set(ZLIB_INCLUDE_DIRS
		"${CMAKE_CURRENT_SOURCE_DIR}/submodules/zlib"
		"${CMAKE_CURRENT_BINARY_DIR}/submodules/zlib"
	)
	set(ZLIB_LIBRARIES zlibstatic)

	set(PNG_BUILD_ZLIB ON)
	set(PNG_SHARED OFF)
	set(PNG_EXECUTABLES OFF)
	set(PNG_TESTS OFF)
	add_subdirectory(submodules/libpng)
	target_include_directories(png_static PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/submodules/libpng"
		"${CMAKE_CURRENT_BINARY_DIR}/submodules/libpng"
	)
	add_library(PNG::PNG ALIAS png_static)

	add_subdirectory(submodules/glm)
	add_subdirectory(submodules/SDL)
	include(imgui)
endif()

if(NOT AFGE_USE_SUBMODULES)
	find_package(PNG REQUIRED)
	find_package(SDL2 CONFIG REQUIRED)
	find_package(glad REQUIRED)
	find_package(glm CONFIG REQUIRED)
endif()

add_subdirectory(modules)
add_subdirectory(engine)

if(AFGE_BUILD_TOOLS)
	if(NOT AFGE_USE_SUBMODULES)
		find_package(imgui CONFIG REQUIRED)
	endif()
	#Not supported by non-submodule build
	add_subdirectory(submodules/nfd) 

	add_subdirectory(tools)
endif()

