local sqlite_dir = "../sqlite3"
local json_dir   = "../json-develop/include"

if os.target()=="windows" then
	include(sqlite_dir)
end

project("ygopen")
	kind("StaticLib")
	warnings("Extra")
	files({"**.hpp", "**.cpp"})
	links("sqlite3")

	filter("system:windows")
		includedirs({sqlite_dir, json_dir})
		defines({ "WIN32", "_WIN32", "NOMINMAX" })

	filter("action:vs*")
		characterset("ASCII")

	filter("system:not windows")
		buildoptions({"-pedantic", "-std=c++11"})
		links("dl")

	filter("system:macosx")
		includedirs(json_dir)
