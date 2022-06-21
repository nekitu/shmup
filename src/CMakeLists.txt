# to the root binary directory of the project as ${HELLO_BINARY_DIR}.
cmake_minimum_required(VERSION 3.0.0)
include(../CMakeUtils.cmake)

add_source_group(src ./ SRCS)
add_source_group(engine ./engine ENGINE_SRCS)
add_source_group(engine ./engine/resources ENGINE_RES_SRCS)
add_source_group(luaintf ../libs/luaintf/luaintf/src LUAINTF_SRCS)
add_source_group(binpack ../libs/binpack BINPACK_SRCS)
add_source_group(lua ../libs/lua LUA_SRCS)

get_filename_component(full_path ${CMAKE_CURRENT_SOURCE_DIR}/../libs/lua/onelua.c ABSOLUTE)
message("${full_path}")

list(REMOVE_ITEM LUA_SRCS "${full_path}")
list(REMOVE_ITEM LUA_SRCS "/home/nekitu/work/shmup/src/../libs/lua/onelua.c")
list(REMOVE_ITEM LUA_SRCS "/home/nekitu/work/shmup/src/../libs/lua/lua.c")
message(${LUA_SRCS})

add_executable(shmup ${SRCS} ${BINPACK_SRCS} ${ENGINE_SRCS} ${ENGINE_RES_SRCS} ${LUAINTF_SRCS} ${LUA_SRCS})

target_include_directories(shmup PUBLIC
	./engine
	../libs/sdl/include
	../libs/sdlmixer/include
	../libs/glew/include
	../libs/jsoncpp/include
	../libs/stb
	../libs/utfcpp/source
	../libs/lua
	../libs/luaintf
	../libs/spdlog/include
	../libs)

find_package(GTK REQUIRED)
find_package(PkgConfig)
pkg_check_modules(GTK "gtk+-3.0")
target_link_libraries(shmup ${GTK_LIBRARIES})
add_definitions(${GTK_CFLAGS} ${GTK_CFLAGS_OTHER} -DMAKE_LIB)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lstdc++fs")
target_link_directories(shmup PUBLIC ${CMAKE_BINARY_DIR}/bin)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
	message("debug mode")
	target_link_libraries(shmup libSDL2-2.0d.so libSDL2_mixer-2.0d.so libGLEWd libjsoncpp libGL.so libspdlogd.so)
else(CMAKE_BUILD_TYPE MATCHES DEBUG)
	target_link_libraries(shmup libSDL2-2.0.so libSDL2_mixer-2.0.so libGLEW.so libjsoncpp.so libGL.so libspdlog.so)
endif(CMAKE_BUILD_TYPE MATCHES DEBUG)

