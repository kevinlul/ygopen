local sqlite_dir = "../sqlite3"
local json_dir = "../json-develop/include"

project("ygopen")
	kind("StaticLib")
	flags("ExtraWarnings")
	files({"**.hpp", "**.cpp"})
	links({"sqlite3"})

	configuration "windows"
		includedirs (sqlite_dir)
		includedirs (json_dir)
		defines { "WIN32", "_WIN32", "NOMINMAX" }
		
	configuration "vs*"
		characterset("MBCS")
	configuration("not windows")
		buildoptions("-pedantic")
		links("dl")
