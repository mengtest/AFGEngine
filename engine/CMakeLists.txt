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
	util.cpp
	window.cpp
	render_context.cpp
	sprite.cpp
)

target_link_libraries(AFGEngine PRIVATE
	#Internal
	FixedPoint
	Geometry
	Image
	Common

	#External
	sol2::sol2
)

#External
target_link_libraries(AFGEngine PRIVATE glm::glm)
target_link_libraries(AFGEngine PRIVATE glad::glad)
target_link_libraries(AFGEngine PRIVATE SDL2::SDL2 SDL2::SDL2main)

#install(TARGETS AFGEngine RUNTIME DESTINATION debug)