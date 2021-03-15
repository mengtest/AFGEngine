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
)

#External
target_link_libraries(AFGEngine PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_link_libraries(AFGEngine PRIVATE glad::glad)
target_link_libraries(AFGEngine PRIVATE PNG::PNG)

#Internal
target_link_libraries(AFGEngine PRIVATE RTAUDIO)
target_link_libraries(AFGEngine PRIVATE FixedPoint)
target_link_libraries(AFGEngine PRIVATE Geometry)

install(TARGETS AFGEngine RUNTIME DESTINATION debug)