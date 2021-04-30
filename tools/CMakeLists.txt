#Image packer dicing tool
add_executable(PackImage)

target_link_libraries(PackImage PRIVATE Geometry)
target_link_libraries(PackImage PRIVATE Image)

target_include_directories(PackImage PRIVATE "./packImage")
target_sources(PackImage PRIVATE
	packImage/pack_image.cpp
)

#Visual framedata editor
add_executable(Frametool)

find_package(imgui CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(glad REQUIRED)
find_package(glm CONFIG REQUIRED)
target_link_libraries(Frametool PRIVATE
	imgui::imgui
	SDL2::SDL2
	SDL2::SDL2main
	glad::glad
	glm::glm
)

target_sources(Frametool PRIVATE
	frametool/main.cpp
)