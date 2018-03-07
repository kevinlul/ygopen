project "sdlpro"
	kind("WindowedApp")

	files({"*.hpp", "*.cpp"})

	buildoptions { "-Wall", "-Wextra", "-pedantic" }
	links({"sqlite3", "SDL2_image", "SDL2_ttf", "SDL2"})

	configuration("not windows")
		includedirs({"/usr/include/SDL2"})
