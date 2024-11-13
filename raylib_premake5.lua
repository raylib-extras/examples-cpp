
function platform_defines()
    defines{"PLATFORM_DESKTOP"}

    filter {"options:graphics=opengl43"}
        defines{"GRAPHICS_API_OPENGL_43"}

    filter {"options:graphics=opengl33"}
        defines{"GRAPHICS_API_OPENGL_33"}

    filter {"options:graphics=opengl21"}
        defines{"GRAPHICS_API_OPENGL_21"}

    filter {"options:graphics=opengl11"}
        defines{"GRAPHICS_API_OPENGL_11"}

    filter {"system:macosx"}
        disablewarnings {"deprecated-declarations"}

    filter {"system:linux"}
        defines {"_GLFW_X11"}
        defines {"_GNU_SOURCE"}
        
    filter{}
end

function get_raylib_dir()
    if (os.isdir("raylib-master")) then
        return "raylib-master"
    end
    if (os.isdir("../raylib-master")) then
        return "../raylib-master"
    end
    if (os.isdir("../raylib")) then
        return "../raylib"
    end
    return "raylib"
end

function link_raylib()
    links {"raylib"}

    raylib_dir = get_raylib_dir();
    includedirs {raylib_dir .. "/src" }
	includedirs {raylib_dir .."/src/external" }
	includedirs {raylib_dir .."/src/external/glfw/include" }
    platform_defines()

    libdirs {"_bin/%{cfg.buildcfg}"}

    filter "action:vs*"
        defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
        dependson {"raylib"}
        links {"raylib.lib"}
        characterset ("MBCS")

    filter "system:windows"
        defines{"_WIN32"}
        links {"winmm", "kernel32", "opengl32", "gdi32"}
        libdirs {"../_bin/%{cfg.buildcfg}"}

    filter "system:linux"
        links {"pthread", "GL", "m", "dl", "rt", "X11"}

    filter "system:macosx"
        links {"OpenGL.framework", "Cocoa.framework", "IOKit.framework", "CoreFoundation.framework", "CoreAudio.framework", "CoreVideo.framework"}

    filter{}
end

function include_raylib()
    raylib_dir = get_raylib_dir();
    includedirs {raylib_dir .."/src" }
	includedirs {raylib_dir .."/src/external" }
	includedirs {raylib_dir .."/src/external/glfw/include" }
    platform_defines()

    filter "action:vs*"
        defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}

    filter{}
end

function defineRaylibProject()
    project "raylib"
        kind "StaticLib"

        platform_defines()

        location "_build"
        language "C"
        targetdir "_bin/%{cfg.buildcfg}"

        filter "action:vs*"
            defines{"_WINSOCK_DEPRECATED_NO_WARNINGS", "_CRT_SECURE_NO_WARNINGS"}
            characterset ("MBCS")

        filter{}

        raylib_dir = get_raylib_dir();
        print ("Using raylib dir " .. raylib_dir);
        includedirs {raylib_dir .. "/src", raylib_dir .. "/src/external/glfw/include" }
        vpaths
        {
            ["Header Files"] = { raylib_dir .. "/src/**.h"},
            ["Source Files/*"] = { raylib_dir .. "/src/**.c"},
        }
        files {raylib_dir .. "/src/*.h", raylib_dir .. "/src/*.c"}

        
        removefiles {raylib_dir .. "/src/rcore_*.c"}


        filter { "system:macosx", "files:" .. raylib_dir .. "/src/rglfw.c" }
            compileas "Objective-C"

        filter{}
end
