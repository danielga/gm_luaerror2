-- Change this variable to the path of your Garry's Mod module base include folder
--local GARRYSMOD_INCLUDES_PATH = "/home/daniel/Área de Trabalho/gmod-module-base/include" -- Linux
--local GARRYSMOD_INCLUDES_PATH = "D:/garrysmod stuff/gmod-module-base/include" -- Windows

-- Change this variable to the path of your SourceSDK folder
--local SOURCE_SDK_PATH = "/home/daniel/Área de Trabalho/SourceSDK" -- Linux
--local SOURCE_SDK_PATH = "D:/garrysmod stuff/SourceSDK" -- Windows

if not GARRYSMOD_INCLUDES_PATH or not SOURCE_SDK_PATH then
	error("GARRYSMOD_INCLUDES_PATH or SOURCE_SDK_PATH not found!")
end

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
			defines({"_LINUX"}) -- lolhackedsourcesdk
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
		defines({"LUAERROR_SERVER", "GAME_DLL", "GMMODULE"})
		includedirs({SOURCE_SDK_PATH .. "/public", SOURCE_SDK_PATH .. "/public/tier0", SOURCE_SDK_PATH .. "/public/tier1", GARRYSMOD_INCLUDES_PATH})
		links(lib_files)
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		targetprefix("gmsv_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")
		if os.is("windows") then
			libdirs({SOURCE_SDK_PATH .. "/lib/public"})
			links({"tier0", "tier1"})
			targetsuffix("_win32")
		elseif os.is("linux") then
			libdirs({"./"})
			linkoptions({"-l:libtier0_srv.so", "-l:tier1_i486.a"})
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY
		elseif os.is("macosx") then
			libdirs({"./"})
			linkoptions({"-l:libtier0.dylib", "-l:tier1_i486.a"})
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY
		end

	project("gmcl_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_CLIENT", "GAME_DLL", "GMMODULE"})
		includedirs({SOURCE_SDK_PATH .. "/public", SOURCE_SDK_PATH .. "/public/tier0", SOURCE_SDK_PATH .. "/public/tier1", GARRYSMOD_INCLUDES_PATH})
		links(lib_files)
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		targetprefix("gmcl_") -- Just to remove prefixes like lib from Linux
		targetname("luaerror2")
		if os.is("windows") then
			libdirs({SOURCE_SDK_PATH .. "/lib/public"})
			links({"tier0", "tier1"})
			targetsuffix("_win32")
		elseif os.is("linux") then
			libdirs({"./"})
			linkoptions({"-l:libtier0_srv.so", "-l:tier1_i486.a"})
			targetsuffix("_linux")
			targetextension(".dll") -- Derp Garry, WHY
		elseif os.is("macosx") then
			libdirs({"./"})
			linkoptions({"-l:libtier0.dylib", "-l:tier1_i486.a"})
			targetsuffix("_mac")
			targetextension(".dll") -- Derp Garry, WHY
		end
