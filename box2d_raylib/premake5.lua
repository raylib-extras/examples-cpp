baseName = path.getbasename(os.getcwd())

defineWorkspace(baseName)

project (baseName)
    files {"../box2d-main/include/**h", "../box2d-main/src/**.cpp", "../box2d-main/src/**.h"}
    includedirs {"../box2d-main/include/"}
    files {"../box2d/include/**h", "../box2dn/src/**.cpp", "../box2d/src/**.h"}
    includedirs {"../box2d/include/"}