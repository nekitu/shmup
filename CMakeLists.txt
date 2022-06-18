cmake_minimum_required(VERSION 3.0.0)
project(shmup)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS}")
add_definitions(-DSDL_MAIN_HANDLED)

if(WINDOWS)
	link_libraries("shlwapi"  "Ws2_32" "Wininet" "dbghelp"
      "user32" "gdi32" "ole32" "oleaut32"  "uuid" "opengl32")
endif(WINDOWS)

if(LINUX)
	link_libraries("X11 `pkg-config --libs gtk+-3.0`"
      "Xi" "dl" "pthread" "Xext" "GL" "GLU")
endif(LINUX)

if(APPLE)
	link_libraries("dl" "ForceFeedback.framework" "CoreVideo.framework" "Cocoa.framework"
      "IOKit.framework" "Carbon.framework" "CoreAudio.framework" "AudioToolbox.framework" "OpenGL.framework")
endif(APPLE)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fPIC" )

link_directories(${CMAKE_BINARY_DIR}/lib ${CMAKE_BINARY_DIR}/../bin)

add_subdirectory(libs)
add_subdirectory(src)