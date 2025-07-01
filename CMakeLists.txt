cmake_minimum_required(VERSION 3.25.0)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(TitorGameEngine LANGUAGES CXX C)

add_subdirectory(dependencies/libFGL)

PreSetup()

#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/data)

#Enable cmake_modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")

message("-- CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_BUILD_TYPE)

add_subdirectory(dependencies/vma)
include(dependencies/glfw)
include(dependencies/glm)
include(cmake_modules/dependencies/tracy.cmake)
include(dependencies/vulkan)
include(dependencies/catch2)
include(dependencies/slang)
include(dependencies/json)

add_subdirectory(src)

PostSetup()
