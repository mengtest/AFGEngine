#Image packer dicing tool
add_executable(PackImage)

target_link_libraries(PackImage PRIVATE Geometry)
target_link_libraries(PackImage PRIVATE Image)

target_include_directories(PackImage PRIVATE "./packImage")
target_sources(PackImage PRIVATE
	packImage/pack_image.cpp
)

#Visual framedata editor
add_subdirectory(frametool)