project("core")
	kind("SharedLib")

	files({"*.cpp", "*.h"})
	defines({"LUA_COMPAT_5_2"})
	buildoptions({ "-Wall", "-Wextra", "-pedantic" })
	links({"lua5.3"})

	configuration("not windows")
		buildoptions({ "-std=c++11", "-Wno-unused-parameter" })
		includedirs({"/usr/include/lua5.3"})
