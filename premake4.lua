local GARRYSMOD_INCLUDES_PATH = "gmod-module-base/include"

solution("gm_luaerror2")

	language("C++")
	location("Projects/" .. os.get() .. "-" .. _ACTION)
	flags({"NoPCH", "StaticRuntime", "EnableSSE"})

	local lib_folder = "public"
	if os.is("macosx") then
		lib_folder = "mac"
		platforms({"Universal32"})
	else
		if os.is("linux") then
			lib_folder = "linux"
		end
		platforms({"x32"})
	end
	configurations({"Release"})

	configuration("Release")
		defines({"NDEBUG"})
		flags({"Optimize"})
		targetdir("Projects/Release")
		objdir("Projects/Intermediate")

	project("gmsv_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_SERVER", "GMMODULE"})
		includedirs({GARRYSMOD_INCLUDES_PATH})
		links(lib_files)
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		targetprefix("gmsv_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")
		if os.is("windows") then
			targetsuffix("_win32")
		elseif os.is("linux") then
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY
		elseif os.is("macosx") then
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY
		end

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_CLIENT", "GMMODULE"})
		includedirs({GARRYSMOD_INCLUDES_PATH})
		links(lib_files)
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		targetprefix("gmcl_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")
		if os.is("windows") then
			targetsuffix("_win32")
		elseif os.is("linux") then
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY
		elseif os.is("macosx") then
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY
		end
