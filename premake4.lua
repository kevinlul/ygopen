solution("OpenYugi")
	location(".")
	language("C++")
	objdir("obj")

	configurations({"Debug", "Release"})
	
	configuration("Debug")
		flags("Symbols")
		defines("_DEBUG")
		targetdir("bin/debug")
	
	configuration("Release")
		flags("OptimizeSpeed")
		defines("_RELEASE")
		targetdir("bin/release")
	
	include("core")
	include("sdlpro")
