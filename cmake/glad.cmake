add_library(glad
	"${PROJECT_SOURCE_DIR}/submodules/glad/src/glad.c"
)

target_include_directories(glad PUBLIC
	"${PROJECT_SOURCE_DIR}/submodules/glad/include"
)

add_library(glad::glad ALIAS glad)