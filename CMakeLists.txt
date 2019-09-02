cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find all dependencies
find_package(Protobuf 3 REQUIRED)

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
	protobuf::libprotobuf
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
)

set(SQLITE3_SOURCE ${CSR}/sqlite3.c ${CSR}/sqlite3.h)
set_source_files_properties(${SQLITE3_SOURCE} PROPERTIES
	LANGUAGE C
)
set_property(SOURCE ${SQLITE3_SOURCE} APPEND PROPERTY
	COMPILE_OPTIONS $<${CLANG_OR_GNU}:-w> $<$<CXX_COMPILER_ID:MSVC>:/W0>
)
target_sources(ygopen PRIVATE
	${SQLITE3_SOURCE}
)

target_sources(ygopen PUBLIC
	${CSR}/banlist.hpp
	${CSR}/core_auxiliary.hpp
	${CSR}/core_interface.hpp
	${CSR}/database_manager.hpp
	${CSR}/deck.hpp
	${CSR}/pod_card.hpp
)
unset(CSR)

# Add protobuf generated source files
protobuf_generate_cpp(PROTOBUF_SRCS PROTOBUF_HDRS
	banlist.proto
	core_data.proto
	core_msg_request.proto
	core_msg_information.proto
	core_msg.proto
	deck.proto
)

set_source_files_properties(${PROTOBUF_SRCS} PROPERTIES
	COMPILE_OPTIONS $<${CLANG_OR_GNU}:-Wno-maybe-uninitialized>
)

target_include_directories(ygopen PUBLIC
	${CMAKE_CURRENT_BINARY_DIR}
)

target_sources(ygopen
	PRIVATE ${PROTOBUF_SRCS}
)

add_subdirectory(codec)
