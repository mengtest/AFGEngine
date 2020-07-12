include_directories("${CMAKE_SOURCE_DIR}/src")

target_sources(AFGEngine PUBLIC
	audio.cpp
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
