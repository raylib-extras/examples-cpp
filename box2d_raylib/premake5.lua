
baseName = path.getbasename(os.getcwd());

project (baseName)
    kind "ConsoleApp"
    location "../build"
    targetdir "../bin/%{cfg.buildcfg}"

    vpaths 
    {
        ["Header Files/*"] = { "**.h"},
        ["Source Files/*"] = {"**.c", "**.cpp"},
    }
    files {"**.c", "**.cpp", "**.h", "../box2d/include/**h", "../box2d/src/**.cpp", "../box2d/src/**.h"}

    includedirs { "./","../box2d/include/"}
	setup_raylib();