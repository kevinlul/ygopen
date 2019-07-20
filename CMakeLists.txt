cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find all dependencies
# option(YGOPEN_COMMON_USE_SYSTEM_JSON "Use system's JSON for Modern C++" ON)
# if(YGOPEN_COMMON_USE_SYSTEM_JSON)
# 	find_package(nlohmann_json REQUIRED)
# else()
# 	set(JSON_BuildTests OFF CACHE INTERNAL "")
# 	# NOTE: assumes that nlohmann::json will be besides this submodule
# 	add_subdirectory(../json)
# endif()

# Add library
add_library(ygopen STATIC)

# Add libraries from dependencies
target_link_libraries(ygopen PUBLIC
	nlohmann_json::nlohmann_json
)

# Set compiler flags
set(CLANG_OR_GNU $<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>)
target_compile_options(ygopen PRIVATE
	# Clang and GNU compilers
	$<${CLANG_OR_GNU}:-Wall -Wextra -pedantic>
	$<$<AND:$<CONFIG:Debug>,${CLANG_OR_GNU}>:-ggdb3 -Og -D_DEBUG>
	$<$<AND:$<CONFIG:Release>,${CLANG_OR_GNU}>:-O2>
	$<$<AND:$<CONFIG:RelWithDebInfo>,${CLANG_OR_GNU}>:-ggdb3 -O2 -D_DEBUG>
	# Microsoft compiler
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Add source files
set(CSR ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(ygopen PRIVATE
	${CSR}/banlist.cpp
	${CSR}/core_auxiliary.cpp
	${CSR}/core_interface.cpp
	${CSR}/database_manager.cpp
	${CSR}/deck.cpp
	# SQLite 3
	${CSR}/sqlite3.c
	${CSR}/sqlite3.h
)

target_sources(ygopen PUBLIC
	${CSR}/banlist.hpp
	${CSR}/core_auxiliary.hpp
	${CSR}/core_interface.hpp
	${CSR}/deck.hpp
	${CSR}/pod_card.hpp
)
unset(CSR)
