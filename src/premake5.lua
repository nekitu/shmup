project "seagle"
    kind "ConsoleApp"
    defines {"_CONSOLE"}
    includedirs {scriptRoot, scriptRoot.."/src/engine"}
    add_sources_from("./")
    link_binpack()
    link_win32()
    link_opengl()
    link_glew()
    link_freetype()
    link_jsoncpp()
    link_stb_image()
    link_sdl2()
    link_lua()

    filter {"system:linux"}
        linkgroups 'On'
        buildoptions {"`pkg-config --cflags gtk+-3.0` -fPIC"}
        links {"X11 `pkg-config --libs gtk+-3.0`"}
        links {"Xi", "dl", "pthread", "Xext"}
    filter {}

    filter {"system:macosx"}
        links {"OpenGL.framework", "ForceFeedback.framework", "CoreVideo.framework", "Cocoa.framework", "IOKit.framework", "Carbon.framework", "CoreAudio.framework", "AudioToolbox.framework", "dl"}
    filter {}

    configuration "Debug"
        defines {}
        symbols "On"
        targetname "seagle_d"

    configuration "Development"
        defines
        {
            "NDEBUG"
        }
        symbols "On"
        targetname "seagle_dev"

    configuration "Release"
        defines
        {
            "NDEBUG"
        }
        optimize "On"
        targetname "seagle"