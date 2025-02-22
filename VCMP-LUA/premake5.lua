project "LuaPlugin"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	pic "On"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin/interm/" .. outputdir .. "/%{prj.name}")
    
	files
	{
		"pch.h",
        "pch.cpp",
        "Core.cpp",

        "include/**.h",
        "include/**.c",
        
        "vcmpWrap/**.h",
		"vcmpWrap/**.cpp",
    }
    
	includedirs
	{
		"%{wks.location}/VCMP-LUA/",
		"%{wks.location}/VCMP-LUA/include",
		"%{wks.location}/VCMP-LUA/vcmpWrap",
		"%{wks.location}/VCMP-LUA/vendor",
		"%{wks.location}/VCMP-LUA/vendor/Lua",
		"%{wks.location}/VCMP-LUA/vendor/sol",
		"%{wks.location}/VCMP-LUA/vendor/spdlog/include",
		"%{wks.location}/VCMP-LUA/vendor/asyncplusplus/include",
		"%{wks.location}/VCMP-LUA/vendor/lanes/src",
	}

	-- Module(s) files and dirs
	files
	{
		-- crypto
		"modules/crypto/vcmpWrap/**.h",
		"modules/crypto/vcmpWrap/**.cpp",
	}

	includedirs
	{
		-- sqlite
		"modules/sqlite3/sqliteCpp/include",
		-- cpr
		"modules/requests/cpr/include"
	}
	--

	defines { "LIBASYNC_STATIC" }

	filter {}
		internalLinks = { "spdlog", "Lua", "asyncplusplus", "LuaLanes" }
		-- Links external modules
		externalLinks = { "module-crypto", "module-sqliteCpp", "module-cpr" }
		
		local linksTbl = table.join(internalLinks, externalLinks)
		links(linksTbl)

	-- Specify internal modules (Not project based)
	include "modules/mariadb"

	filter "system:windows"
		systemversion "latest"
		defines { "WIN32" }
	  
	filter "configurations:Debug"
		defines {"_DEBUG"}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines {"_RELEASE"}
		runtime "Release"
		optimize "on"