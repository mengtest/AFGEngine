#RtAudio
add_library(RTAUDIO STATIC RtAudio/RtAudio.cpp)

if(WIN32)
add_compile_definitions(__WINDOWS_DS__)
target_link_libraries(RTAUDIO ole32 winmm dsound)
endif()

target_include_directories(RTAUDIO PUBLIC RtAudio)

#FixedPoint
add_library(FixedPoint STATIC fixed_point/fixed_point.cpp)
target_compile_definitions(FixedPoint PUBLIC FP_FRACBITS=16)
target_include_directories(FixedPoint PUBLIC fixed_point)

#Geometry
add_library(Geometry INTERFACE)
target_include_directories(Geometry INTERFACE geometry)


