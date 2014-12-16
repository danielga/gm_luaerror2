GARRYSMOD_MODULE_BASE_FOLDER = "../gmod-module-base"
SCANNING_FOLDER = "../scanning"
DETOURING_FOLDER = "../detouring"
SOURCE_FOLDER = "../Source"
PROJECT_FOLDER = os.get() .. "/" .. _ACTION

solution("gm_luaerror2")
	language("C++")
	location(PROJECT_FOLDER)
	warnings("Extra")
	flags({"NoPCH", "StaticRuntime"})
	platforms({"x86"})
	configurations({"Release", "Debug"})

	filter("platforms:x86")
		architecture("x32")

	filter("configurations:Release")
		optimize("On")
		vectorextensions("SSE2")
		objdir(PROJECT_FOLDER .. "/Intermediate")
		targetdir(PROJECT_FOLDER .. "/Release")

	filter({"configurations:Debug"})
		flags({"Symbols"})
		objdir(PROJECT_FOLDER .. "/Intermediate")
		targetdir(PROJECT_FOLDER .. "/Debug")

	project("gmsv_luaerror2")
		kind("SharedLib")
		defines({"GMMODULE", "LUAERROR_SERVER"})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SCANNING_FOLDER,
			DETOURING_FOLDER
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SCANNING_FOLDER .. "/SymbolFinder.cpp",
			DETOURING_FOLDER .. "/hde.cpp"
		})
		vpaths({
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SCANNING_FOLDER .. "/**.cpp",
				DETOURING_FOLDER .. "/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			targetsuffix("_win32")

		filter("system:linux")
			links({"dl"})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			links({"dl"})
			buildoptions({"-std=c++11"})
			targetsuffix("_mac")

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"GMMODULE", "LUAERROR_CLIENT"})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SCANNING_FOLDER,
			DETOURING_FOLDER
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SCANNING_FOLDER .. "/SymbolFinder.cpp",
			DETOURING_FOLDER .. "/hde.cpp"
		})
		vpaths({
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SCANNING_FOLDER .. "/**.cpp",
				DETOURING_FOLDER .. "/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			targetsuffix("_win32")

		filter("system:linux")
			links({"dl"})
			buildoptions({"-std=c++11"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			links({"dl"})
			buildoptions({"-std=c++11"})
			targetsuffix("_mac")
