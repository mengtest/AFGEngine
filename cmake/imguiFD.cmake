SET(imguiFD_PATH "${PROJECT_SOURCE_DIR}/submodules/ImGuiFileDialog")

if(NOT EXISTS "${imguiFD_PATH}/ImGuiFileDialog.h")
	message(FATAL_ERROR "ImGuiFileDialog not found. Did you forget to init the submodules?")
endif()

file(GLOB imguiFD_CPP
	"${imguiFD_PATH}/*.cpp"
)

add_library(imguiFD
	${imguiFD_CPP}
)

target_include_directories(imguiFD PUBLIC
	"${imguiFD_PATH}"
	"${PROJECT_SOURCE_DIR}/submodules"
)

target_compile_definitions(imguiFD PRIVATE CUSTOM_IMGUIFILEDIALOG_CONFIG=<imguiFD_config.h> )
target_link_libraries(imguiFD PRIVATE imgui::imgui)