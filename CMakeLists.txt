cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Protobuf 3 REQUIRED)
include_directories(${Protobuf_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

protobuf_generate_cpp(OCGCORE_PROTO_SRCS OCGCORE_PROTO_HDRS
                      core_data.proto core_msg_request.proto
                      core_msg_information.proto core_msg.proto)

set(OCGCORE_ENCODER_HDRS encoder_impl_begin.hpp encoder_impl_end.hpp
    encoder_information_impl.hpp encoder_request_impl.hpp
    encoder_spec_information_impl.hpp)

add_library(ocgcore-proto STATIC msg_encoder.cpp msg_codec.hpp buffer.hpp
            ${OCGCORE_ENCODER_HDRS})
target_link_libraries(ocgcore-proto ${Protobuf_LIBRARIES})

# Enable great deal of warnings
if(MSVC)
  target_compile_options(ocgcore-proto PRIVATE /W4 /WX)
else()
  target_compile_options(ocgcore-proto PRIVATE -Wall -Wextra -pedantic)
endif()
