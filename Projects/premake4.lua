GARRYSMOD_INCLUDES_PATH = "../gmod-module-base/include"
PROJECT_FOLDER = os.get() .. "/" .. _ACTION
SOURCE_FOLDER = "../Source/"

solution("gm_luaerror2")
	language("C++")
	location(PROJECT_FOLDER)
	flags({"NoPCH", "ExtraWarnings"})

	if os.is("macosx") then
		platforms({"Universal32"})
	else
		platforms({"x32"})
	end

	configurations({"Debug", "Release"})

	configuration("Debug")
		flags({"Symbols"})
		targetdir(PROJECT_FOLDER .. "/Debug")
		objdir(PROJECT_FOLDER .. "/Intermediate")

	configuration("Release")
		flags({"Optimize", "EnableSSE"})
		targetdir(PROJECT_FOLDER .. "/Release")
		objdir(PROJECT_FOLDER .. "/Intermediate")

	project("gmsv_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_SERVER", "GMMODULE"})
		includedirs({SOURCE_FOLDER, GARRYSMOD_INCLUDES_PATH})
		files({SOURCE_FOLDER .. "*.cpp", SOURCE_FOLDER .. "*.hpp", SOURCE_FOLDER .. "MologieDetours/hde.cpp", SOURCE_FOLDER .. "MologieDetours/detours.h"})
		vpaths({["Header files/*"] = {SOURCE_FOLDER .. "**.hpp", SOURCE_FOLDER .. "**.h"}, ["Source files/*"] = {SOURCE_FOLDER .. "**.cpp", SOURCE_FOLDER .. "MologieDetours/**.cpp"}})
		
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
		includedirs({SOURCE_FOLDER, GARRYSMOD_INCLUDES_PATH})
		files({SOURCE_FOLDER .. "*.cpp", SOURCE_FOLDER .. "*.hpp", SOURCE_FOLDER .. "MologieDetours/hde.cpp", SOURCE_FOLDER .. "MologieDetours/detours.h"})
		vpaths({["Header files/*"] = {SOURCE_FOLDER .. "**.hpp", SOURCE_FOLDER .. "**.h"}, ["Source files/*"] = {SOURCE_FOLDER .. "**.cpp", SOURCE_FOLDER .. "MologieDetours/**.cpp"}})

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