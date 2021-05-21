#FixedPoint
add_library(FixedPoint STATIC fixed_point/fixed_point.cpp)
target_compile_definitions(FixedPoint PUBLIC FP_FRACBITS=16)
target_include_directories(FixedPoint PUBLIC fixed_point)

#Geometry
add_library(Geometry INTERFACE)
target_include_directories(Geometry INTERFACE geometry)

#A libpng wrapper to load and write images
add_library(Image SHARED image/image.cpp)
set_target_properties(Image PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
target_compile_definitions(Image PRIVATE LIB_PNG_WARN=0)
target_include_directories(Image PUBLIC image)

if(NOT AFGE_USE_SUBMODULES)
	find_package(PNG REQUIRED)
endif()
target_link_libraries(Image PUBLIC PNG::PNG)