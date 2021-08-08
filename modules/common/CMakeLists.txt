add_library(Common STATIC)

target_include_directories(Common PUBLIC ".")
target_sources(Common PRIVATE
	shader.cpp
	texture.cpp
	vao.cpp
	ubo.cpp
	gfx_handler.cpp
)

target_link_libraries(Common PUBLIC Image lz4_static glad::glad)
target_link_libraries(Common PRIVATE sol2::sol2)
