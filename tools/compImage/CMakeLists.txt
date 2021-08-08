#Image packer dicing tool
add_executable(compImage)
target_link_libraries(compImage PRIVATE
	Image
	squish
)

#target_include_directories(PackImage PRIVATE ".")
target_sources(compImage PRIVATE
	main.cpp
)
