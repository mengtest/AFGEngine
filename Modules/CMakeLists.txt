#RtAudio
add_library(RTAUDIO STATIC RtAudio/RtAudio.cpp)
target_include_directories(RTAUDIO PUBLIC RtAudio)

if(WIN32)
add_compile_definitions(__WINDOWS_DS__)
target_link_libraries(RTAUDIO PRIVATE ole32 winmm dsound)
endif()

#FixedPoint
add_library(FixedPoint STATIC fixed_point/fixed_point.cpp)
target_compile_definitions(FixedPoint PUBLIC FP_FRACBITS=16)
target_include_directories(FixedPoint PUBLIC fixed_point)

#Geometry
add_library(Geometry INTERFACE)
target_include_directories(Geometry INTERFACE geometry)

#A libpng wrapper to load and write images
add_library(Image STATIC image/image.cpp)
target_compile_definitions(Image PRIVATE LIB_PNG_WARN=1)
target_include_directories(Image PUBLIC image)
target_link_libraries(Image PRIVATE PNG::PNG)