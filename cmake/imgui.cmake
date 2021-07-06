SET(imgui_PATH "${PROJECT_SOURCE_DIR}/submodules/imgui")

if(NOT EXISTS "${imgui_PATH}/imgui.h")
	message(FATAL_ERROR "Imgui not found. Did you forget to init the submodules?")
endif()

file(GLOB imgui_CPP
	"${imgui_PATH}/*.cpp"
)

add_library(imgui
	"${imgui_PATH}/backends/imgui_impl_opengl3.cpp"
	"${imgui_PATH}/backends/imgui_impl_sdl.cpp"
	"${imgui_PATH}/misc/cpp/imgui_stdlib.cpp"
	${imgui_CPP}
)

target_include_directories(imgui PUBLIC
	"${imgui_PATH}"
	"${imgui_PATH}/backends"
	"${imgui_PATH}/misc/cpp"
	"${PROJECT_SOURCE_DIR}/submodules"
)


target_compile_definitions(imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD IMGUI_USER_CONFIG=<imgui_config.h> )
target_link_libraries(imgui PRIVATE glad SDL2::SDL2)
add_library(imgui::imgui ALIAS imgui)
