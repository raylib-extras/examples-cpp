
newoption
{
    trigger = "graphics",
    value = "OPENGL_VERSION",
    description = "version of OpenGL to build raylib against",
    allowed = {
	    { "opengl11", "OpenGL 1.1"},
	    { "opengl21", "OpenGL 2.1"},
	    { "opengl33", "OpenGL 3.3"},
	    { "opengl43", "OpenGL 4.3"}
    },
    default = "opengl33"
}

function define_C()
    language "C"
end

function define_Cpp()
    language "C++"
end

function string.starts(String,Start)
    return string.sub(String,1,string.len(Start))==Start
end

function link_to(lib)
    links (lib)
    includedirs ("../"..lib,"../"..lib.."/include" )
end

function download_progress(total, current)
    local ratio = current / total;
    ratio = math.min(math.max(ratio, 0), 1);
    local percent = math.floor(ratio * 100);
    print("Download progress (" .. percent .. "%/100%)")
  end

function check_raylib()
    if(os.isdir("raylib") == false and os.isdir("raylib-master") == false) then
        if(not os.isfile("raylib-master.zip")) then
            print("Raylib not found, downloading from github")
            local result_str, response_code = http.download("https://github.com/raysan5/raylib/archive/refs/heads/master.zip", "raylib-master.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("raylib-master.zip", os.getcwd())
        os.remove("raylib-master.zip")
    end
end

function check_box2d()
    if(os.isdir(".box2d") == false and os.isdir("box2d-main") == false) then
        if(not os.isfile("box2d-main.zip")) then
            print("box2d not found, downloading from github")
            local result_str, response_code = http.download("https://github.com/erincatto/box2d/archive/refs/heads/main.zip", "box2d-main.zip", {
                progress = download_progress,
                headers = { "From: Premake", "Referer: Premake" }
            })
        end
        print("Unzipping to " ..  os.getcwd())
        zip.extract("box2d-main.zip", os.getcwd())
        os.remove("box2d-main.zip")
    end
end

function defineWorkspace(baseName)
    workspace (baseName)
        configurations { "Debug", "Release"}
        platforms { "x64", "x86"}

        filter "configurations:Debug"
            defines { "DEBUG" }
            symbols "On"

        filter "configurations:Release"
            defines { "NDEBUG" }
            optimize "On"

        filter { "platforms:x64" }
            architecture "x86_64"

        filter {}
        
        targetdir "_bin/%{cfg.buildcfg}/"

        startproject(baseName)

    defineRaylibProject()

    project (baseName)
        kind "ConsoleApp"
        location "_build"
        targetdir "_bin/%{cfg.buildcfg}"

        filter "action:vs*"
            debugdir "$(SolutionDir)"
		
	    filter {"action:vs*", "configurations:Release"}
            kind "WindowedApp"
            entrypoint "mainCRTStartup"
            
        filter{}

        vpaths 
        {
            ["Header Files/*"] = { "include/**.h",  "include/**.hpp", "src/**.h", "src/**.hpp", "**.h", "**.hpp"},
            ["Source Files/*"] = {"src/**.c", "src/**.cpp","**.c", "**.cpp"},
        }
        files {"**.c", "**.cpp", "**.h", "**.hpp"}

        includedirs { "./"}
        includedirs { "./src"}
        includedirs { "./include"}
        link_raylib();
end

include ("raylib_premake5.lua")
cdialect "C99"
cppdialect "C++17"
check_raylib()
check_box2d()

folders = os.matchdirs("*")
for _, folderName in ipairs(folders) do
    if (string.starts(folderName, "raylib") == false and string.starts(folderName, "_") == false and string.starts(folderName, ".") == false) then
        if (os.isfile(folderName .. "/premake5.lua")) then
            print(folderName)
            include (folderName)
        end
    end
end
