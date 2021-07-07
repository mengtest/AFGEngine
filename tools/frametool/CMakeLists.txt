#Visual framedata editor
add_executable(Frametool)

if(NOT AFGE_USE_SUBMODULES)
	find_package(imgui CONFIG REQUIRED)
	find_package(SDL2 CONFIG REQUIRED)
	find_package(glad REQUIRED)
	find_package(glm CONFIG REQUIRED)
endif()

target_link_libraries(Frametool PRIVATE
	glad::glad
	glm::glm
	SDL2::SDL2
	SDL2::SDL2main
	imgui::imgui
	nfd
	Common
)

target_sources(Frametool PRIVATE
	main.cpp
	main_frame.cpp
	render.cpp
	window.cpp
	ini.cpp
	framedata.cpp
	main_pane.cpp
	imgui_utils.cpp
)