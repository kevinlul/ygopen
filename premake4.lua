project("ygopen")
	kind("StaticLib")
	flags("ExtraWarnings")
	files({"**.hpp", "**.cpp"})
	links({"sqlite3"})

	configuration("not windows")
		buildoptions("-pedantic")
		links("dl")
