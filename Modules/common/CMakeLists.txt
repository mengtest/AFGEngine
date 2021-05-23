add_library(Common STATIC)

target_include_directories(Common PUBLIC ".")
target_sources(Common PRIVATE
	shader.cpp
	texture.cpp
	vao.cpp
	ubo.cpp
)

target_link_libraries(Common PUBLIC Image glad::glad)
