add_executable(AFGEngine)

target_include_directories(AFGEngine PRIVATE ".")
target_sources(AFGEngine PRIVATE
	resource.rc
	#audio.cpp
	camera.cpp
	chara.cpp
	chara_input.cpp
	hud.cpp
	main.cpp
	#netplay.cpp
	raw_input.cpp
	shader.cpp
	texture.cpp
	util.cpp
	window.cpp
	render_context.cpp
	vao.cpp
	ubo.cpp
)

if(NOT AFGE_USE_SUBMODULES)
	find_package(SDL2 CONFIG REQUIRED)
	find_package(glad REQUIRED)
	find_package(glm CONFIG REQUIRED)
endif()

#Internal
target_link_libraries(AFGEngine PRIVATE FixedPoint)
target_link_libraries(AFGEngine PRIVATE Geometry)
target_link_libraries(AFGEngine PRIVATE Image)

#External
target_link_libraries(AFGEngine PRIVATE glm::glm)
target_link_libraries(AFGEngine PRIVATE glad::glad)
target_link_libraries(AFGEngine PRIVATE SDL2::SDL2 SDL2::SDL2main)

#install(TARGETS AFGEngine RUNTIME DESTINATION debug)