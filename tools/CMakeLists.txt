add_executable(PackImage)



target_include_directories(PackImage PRIVATE ".")
target_sources(PackImage PRIVATE
	pack_image.cpp
)

target_link_libraries(PackImage PRIVATE Image)