project "psd2sheet"
    kind "ConsoleApp"
    defines {"_CONSOLE"}
    includedirs {scriptRoot, scriptRoot.."/tools/psd2sheet/libpng"}
    includedirs {scriptRoot, scriptRoot.."/tools/psd2sheet/zlib"}
    includedirs {scriptRoot, scriptRoot.."/src/engine"}
    includedirs {scriptRoot, scriptRoot.."/libs/jsoncpp/include"}
    add_sources_from("./")
    add_sources_from("./libpng")
    add_sources_from("./zlib")
    files { "../../src/engine/utils.cpp" }
    link_jsoncpp()


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
        -- target name is the same for all builds, easier for cmd line
        targetname "psd2sheet"

    configuration "Development"
        defines
        {
            "NDEBUG"
        }
        symbols "On"
        targetname "psd2sheet"

    configuration "Release"
        defines
        {
            "NDEBUG"
        }
        optimize "On"
        targetname "psd2sheet"