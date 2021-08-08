#Image packer dicing tool
add_executable(PackImage)
target_link_libraries(PackImage PRIVATE
	Geometry
	Image
	args
)

#target_include_directories(PackImage PRIVATE ".")
target_sources(PackImage PRIVATE
	pack_image.cpp
)
