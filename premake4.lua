local GARRYSMOD_INCLUDES_PATH = "gmod-module-base/include"
local project_folder = "Projects/" .. os.get() .. "/" .. _ACTION

solution("gm_luaerror2")

	language("C++")
	location(project_folder)
	flags({"NoPCH", "ExtraWarnings"})

	if os.is("macosx") then
		platforms({"Universal32"})
	else
		platforms({"x32"})
	end

	configurations({"Debug", "Release"})

	configuration("Debug")
		defines({"DEBUG"})
		flags({"Symbols"})
		targetdir(project_folder .. "/Debug")
		objdir(project_folder .. "/Intermediate")

	configuration("Release")
		defines({"NDEBUG"})
		flags({"Optimize", "EnableSSE"})
		targetdir(project_folder .. "/Release")
		objdir(project_folder .. "/Intermediate")

	project("gmsv_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_SERVER", "GMMODULE"})
		includedirs({"Source", GARRYSMOD_INCLUDES_PATH})
		files({"Source/*.cpp", "Source/*.hpp", "Source/MologieDetours/hde.cpp", "Source/MologieDetours/detours.h"})
		vpaths({["Header files/*"] = {"Source/**.hpp", "Source/**.h"}, ["Source files/*"] = {"Source/**.cpp", "Source/MologieDetours/**.cpp"}})
		
		targetprefix("gmsv_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")

		configuration("windows")
			targetsuffix("_win32")

		configuration("linux")
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY

		configuration("macosx")
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_CLIENT", "GMMODULE"})
		includedirs({"Source", GARRYSMOD_INCLUDES_PATH})
		files({"Source/*.cpp", "Source/*.hpp", "Source/MologieDetours/hde.cpp", "Source/MologieDetours/detours.h"})
		vpaths({["Header files/*"] = {"Source/**.hpp", "Source/**.h"}, ["Source files/*"] = {"Source/**.cpp"}})

		targetprefix("gmcl_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")

		configuration("windows")
			targetsuffix("_win32")

		configuration("linux")
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY

		configuration("macosx")
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY