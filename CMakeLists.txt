#set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")
#set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# /CMakeLists.txt
cmake_minimum_required(VERSION 4.0 FATAL_ERROR)
project(Titor LANGUAGES CXX C)

find_program(CCACHE_FOUND ccache)
if (CCACHE_FOUND)
	set(CMAKE_CXX_COMPILER_LAUNCHER ccache)
else ()
	message("ccache not found. Not using it")
endif ()

option(FGL_ENABLE_PROFILING OFF)

project(TitorGameEngine LANGUAGES CXX C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake_modules")
include(dependencies/libFGL)
#add_subdirectory(fgl_cmake_modules)

PreSetup()

#file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/data)

#Enable cmake_modules

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
