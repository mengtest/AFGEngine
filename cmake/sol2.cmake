add_subdirectory("${PROJECT_SOURCE_DIR}/submodules/lua")

add_library(sol2 INTERFACE)
add_library(sol2::sol2 ALIAS sol2)
target_include_directories(sol2 INTERFACE "${PROJECT_SOURCE_DIR}/submodules/sol2")
target_link_libraries(sol2 INTERFACE lua_static)
