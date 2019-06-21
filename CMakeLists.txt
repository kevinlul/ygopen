cmake_minimum_required(VERSION 3.12)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure CMAKE_BUILD_TYPE is set correctly.
if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
string(TOUPPER "CMAKE_CXX_FLAGS_${CMAKE_BUILD_TYPE}" BUILD_TYPE_CXX_FLAGS)

set(PROJECT_NAME ygopen)
set(PROJECT_VERSION 0.1)
set(PROJECT_EXECUTABLE ${PROJECT_NAME})
project(${PROJECT_NAME} CXX)

# TODO: Replay record and discord presence options

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)

add_executable(${PROJECT_EXECUTABLE})
target_include_directories(${PROJECT_EXECUTABLE} PRIVATE ${SDL2_INCLUDE_DIRS}
                           ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_TTF_INCLUDE_DIR})
target_link_libraries(${PROJECT_EXECUTABLE} PRIVATE ocgcore-proto
                      ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES}
                      ${SDL2_TTF_LIBRARIES})

if(MSVC)
# TODO
else()
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic")
	set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-ggdb3 -O2 -DDEBUGGING")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
	set(CMAKE_CXX_FLAGS_DEBUG  "-ggdb3 -Og -DDEBUGGING")
endif()

add_subdirectory(src)
