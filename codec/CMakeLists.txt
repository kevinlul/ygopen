cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf 3 REQUIRED)

# Generate source files
protobuf_generate_cpp(OCGCORE_PROTO_SRCS OCGCORE_PROTO_HDRS
	core_data.proto
	core_msg_request.proto
	core_msg_information.proto
	core_msg.proto
)

# Add library
add_library(ocgcore-proto STATIC)

# Add include directories
target_include_directories(ocgcore-proto PUBLIC
	${Protobuf_INCLUDE_DIRS}
	${CMAKE_CURRENT_BINARY_DIR}
)

# Protobuf should be the only depedency of this project
target_link_libraries(ocgcore-proto PUBLIC
	${Protobuf_LIBRARIES}
)

# Set compiler flags
set(CLANG_OR_GNU $<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>)
target_compile_options(ocgcore-proto PRIVATE
	# Clang and GNU compilers
	$<${CLANG_OR_GNU}:-Wall -Wextra -pedantic>
	$<$<AND:$<CONFIG:Debug>,${CLANG_OR_GNU}>:-ggdb3 -Og -D_DEBUG>
	$<$<AND:$<CONFIG:Release>,${CLANG_OR_GNU}>:-O2>
	$<$<AND:$<CONFIG:RelWithDebInfo>,${CLANG_OR_GNU}>:-ggdb3 -O2 -D_DEBUG>
	# Microsoft compiler
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
)

set_source_files_properties(${OCGCORE_PROTO_SRCS} PROPERTIES
	COMPILE_OPTIONS $<${CLANG_OR_GNU}:-Wno-maybe-uninitialized>
)

# Add source files
set(CSR ${CMAKE_CURRENT_SOURCE_DIR})
set(OCGCORE_ENCODER_HDRS
	${CSR}/encoder_impl_begin.hpp
	${CSR}/encoder_impl_end.hpp
	${CSR}/encoder_information_impl.hpp
	${CSR}/encoder_request_impl.hpp
	${CSR}/encoder_spec_information_impl.hpp
)

target_sources(ocgcore-proto PRIVATE
	${OCGCORE_PROTO_SRCS}
	${CSR}/buffer.hpp
	${CSR}/core_message.hpp
	${OCGCORE_ENCODER_HDRS}
	${CSR}/msg_encoder.cpp
	${CSR}/msgs_map.hpp
)

target_sources(ocgcore-proto PUBLIC
# 	${OCGCORE_PROTO_HDRS} # At configuration stage, files do not exist yet
	${CSR}/msg_codec.hpp
)
unset(CSR)
