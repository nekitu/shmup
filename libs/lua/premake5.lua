project "lua"
  kind "StaticLib"
  buildoptions {
    '-D_WIN32_WINNT=0x501',
  }
  includedirs  {
    'src',
    'luajit/src',
    'luaintf'
  }
  files {
    "lua/src/**.c",
    "lua/src/**.h"
  }
  removefiles {
    'src/luac.c', -- remove the command line utility from the build
    'luajit/src/host/**.c'
  }