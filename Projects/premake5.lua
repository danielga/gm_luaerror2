GARRYSMOD_MODULE_BASE_FOLDER = "../gmod-module-base"
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
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include"
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/MologieDetours/hde.cpp",
			SOURCE_FOLDER .. "/MologieDetours/detours.h"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.hpp",
				SOURCE_FOLDER .. "/**.h"
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SOURCE_FOLDER .. "/MologieDetours/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			targetsuffix("_win32")

		filter("system:linux")
			links({"dl"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			links({"dl"})
			targetsuffix("_mac")

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"GMMODULE", "LUAERROR_CLIENT"})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include"
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/MologieDetours/hde.cpp",
			SOURCE_FOLDER .. "/MologieDetours/detours.h"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.hpp",
				SOURCE_FOLDER .. "/**.h"
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SOURCE_FOLDER .. "/MologieDetours/**.cpp"
			}
		})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			targetsuffix("_win32")

		filter("system:linux")
			links({"dl"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			links({"dl"})
			targetsuffix("_mac")
