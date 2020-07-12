#RtAudio
add_library(RTAUDIO STATIC RtAudio/RtAudio.cpp)

if(WIN32)
add_compile_definitions(__WINDOWS_DS__)
target_link_libraries(RTAUDIO ole32 winmm dsound)
endif()

target_include_directories(RTAUDIO PUBLIC RtAudio)
target_link_libraries(AFGEngine PUBLIC RTAUDIO)

#FixedPoint
add_library(FixedPoint STATIC fixed_point/fixed_point.cpp)
add_compile_definitions(FP_FRACBITS=16)
target_include_directories(FixedPoint PUBLIC fixed_point)
target_link_libraries(AFGEngine PUBLIC FixedPoint)

#Geometry
target_include_directories(AFGEngine PRIVATE geometry)

