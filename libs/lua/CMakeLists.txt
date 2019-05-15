cmake_minimum_required (VERSION 2.6)

set(SAVED_PROJECT "${PROJECT_NAME}")

project(lua)

include_directories("${libDir}/lua/Lua/src")

#FILE(GLOB sources src/*.cpp src/*.h src/*.c)
set(sources
  ${CMAKE_CURRENT_LIST_DIR}/src/lapi.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lauxlib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lbaselib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lbitlib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lcode.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lcorolib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lctype.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ldblib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ldebug.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ldo.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ldump.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lfunc.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lgc.c
  ${CMAKE_CURRENT_LIST_DIR}/src/linit.c
  ${CMAKE_CURRENT_LIST_DIR}/src/liolib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/llex.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lmathlib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lmem.c
  ${CMAKE_CURRENT_LIST_DIR}/src/loadlib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lobject.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lopcodes.c
  ${CMAKE_CURRENT_LIST_DIR}/src/loslib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lparser.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lstate.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lstring.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lstrlib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ltable.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ltablib.c
  ${CMAKE_CURRENT_LIST_DIR}/src/ltm.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lundump.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lvm.c
  ${CMAKE_CURRENT_LIST_DIR}/src/lzio.c
)

add_library(lua ${sources})

SET_TARGET_PROPERTIES(lua PROPERTIES COMPILE_FLAGS "/W0 -D_WIN32_WINNT=0x501 /FI\"${CMAKE_CURRENT_LIST_DIR}/../LuaSandbox/lua_sandbox.h\"")

# back to the other project
project("${SAVED_PROJECT}")
