local GARRYSMOD_INCLUDES_PATH = "gmod-module-base/include"
local SDK_PATH = "E:/Programming/source-sdk-2013/mp/src"

solution("gm_luaerror2")

	language("C++")
	location("Projects/" .. os.get() .. "-" .. _ACTION)
	flags({"NoPCH"})

	if os.is("macosx") then
		platforms({"Universal32"})
	else
		platforms({"x32"})
	end

	configuration("windows")
		libdirs({SDK_PATH .. "/lib/public"})
		linkoptions({"/NODEFAULTLIB:libc", "/NODEFAULTLIB:libcd", "/NODEFAULTLIB:libcmt"})

	configuration("macosx")
		libdirs({SDK_PATH .. "/lib/public/osx32"})
		linkoptions({"-nostdlib"})

	configuration("linux")
		libdirs({SDK_PATH .. "/lib/public/linux32"})
		-- remove standard libraries from build

	configurations({"Release"})

	configuration("Release")
		defines({"NDEBUG"})
		flags({"Optimize", "EnableSSE"})
		targetdir("Projects/Release")
		objdir("Projects/Intermediate")
		links({"tier0", "tier1", "tier2"})

	project("gmsv_luaerror2")
		kind("SharedLib")
		defines({"LUAERROR_SERVER", "GAME_DLL", "GMMODULE"})
		includedirs({GARRYSMOD_INCLUDES_PATH})
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})
		
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
		defines({"LUAERROR_CLIENT", "CLIENT_DLL", "GMMODULE"})
		includedirs({GARRYSMOD_INCLUDES_PATH})
		files({"*.c", "*.cxx", "*.cpp", "*.h", "*.hxx", "*.hpp", "MologieDetours/hde.cpp", "MologieDetours/detours.h", "MologieDetours/hde.h"})
		vpaths({["Header files"] = {"**.h", "**.hxx", "**.hpp"}, ["Source files"] = {"**.c", "**.cxx", "**.cpp"}})

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