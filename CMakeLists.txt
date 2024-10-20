# /CMakeLists.txt

cmake_minimum_required(VERSION 3.25.0)
project(TitorGameEngine LANGUAGES CXX C)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif ()

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MESSAGE_LOG_LEVEL DEBUG CACHE STRING "CMake messaging level")

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/data)

#Enable cmake_modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
include(common)

message("-- CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_BUILD_TYPE)

PlatformPreSetup()
CompilerPreSetup()
message("-- FGL_FLAGS: ${FGL_FLAGS}")

add_subdirectory(dependencies/vma)
include(dependencies/glfw)
include(dependencies/glm)
include(cmake_modules/dependencies/tracy.cmake)
include(dependencies/vulkan)
include(dependencies/catch2)
include(dependencies/shaderc)

add_subdirectory(src)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMENT "Copying build commands to src dir")

SetVersionInfo()
CompilerPostSetup()
