SDK_FOLDER = "E:/Programming/source-sdk-2013/mp/src"
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
		defines({"GMMODULE", "LUAERROR_SERVER", "GAME_DLL"})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SDK_FOLDER .. "/public",
			SDK_FOLDER .. "/public/tier0"
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/MologieDetours/hde.cpp",
			SOURCE_FOLDER .. "/MologieDetours/detours.h",
			SDK_FOLDER .. "/public/tier0/memoverride.cpp"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.hpp",
				SOURCE_FOLDER .. "/**.h"
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SOURCE_FOLDER .. "/MologieDetours/**.cpp",
				SDK_FOLDER .. "/**.cpp"
			}
		})
		links({"tier0", "tier1"})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			defines({"SUPPRESS_INVALID_PARAMETER_NO_INFO"})
			libdirs({SDK_FOLDER .. "/lib/public"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Release"})
				linkoptions({"/NODEFAULTLIB:libcmtd.lib"})

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:libcmt.lib"})

		filter("system:linux")
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32"})
			targetsuffix("_mac")

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"GMMODULE", "LUAERROR_CLIENT", "CLIENT_DLL"})
		includedirs({
			SOURCE_FOLDER,
			GARRYSMOD_MODULE_BASE_FOLDER .. "/include",
			SDK_FOLDER .. "/public",
			SDK_FOLDER .. "/public/tier0"
		})
		files({
			SOURCE_FOLDER .. "/*.cpp",
			SOURCE_FOLDER .. "/*.hpp",
			SOURCE_FOLDER .. "/MologieDetours/hde.cpp",
			SOURCE_FOLDER .. "/MologieDetours/detours.h",
			SDK_FOLDER .. "/public/tier0/memoverride.cpp"
		})
		vpaths({
			["Header files"] = {
				SOURCE_FOLDER .. "/**.hpp",
				SOURCE_FOLDER .. "/**.h"
			},
			["Source files"] = {
				SOURCE_FOLDER .. "/**.cpp",
				SOURCE_FOLDER .. "/MologieDetours/**.cpp",
				SDK_FOLDER .. "/**.cpp"
			}
		})
		links({"tier0", "tier1"})

		targetprefix("")
		targetextension(".dll")

		filter("system:windows")
			defines({"SUPPRESS_INVALID_PARAMETER_NO_INFO"})
			libdirs({SDK_FOLDER .. "/lib/public"})
			targetsuffix("_win32")

			filter({"system:windows", "configurations:Release"})
				linkoptions({"/NODEFAULTLIB:libcmtd.lib"})

			filter({"system:windows", "configurations:Debug"})
				linkoptions({"/NODEFAULTLIB:libcmt.lib"})

		filter("system:linux")
			libdirs({SDK_FOLDER .. "/lib/public/linux32"})
			targetsuffix("_linux")

		filter({"system:macosx"})
			libdirs({SDK_FOLDER .. "/lib/public/osx32"})
			targetsuffix("_mac")