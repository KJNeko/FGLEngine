# /CMakeLists.txt

cmake_minimum_required(VERSION 3.25.0)
project(TitorGameEngine LANGUAGES CXX C)

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
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

add_subdirectory(src)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/shaders")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/assets")

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_UPPER_BUILD_TYPE)

if (CMAKE_UPPER_BUILD_TYPE STREQUAL "DEBUG" OR CMAKE_UPPER_BUILD_TYPE STREQUAL "RELWITHDEBINFO")
	set(SHADER_DEBUG_FLAGS "-g")
else ()
	set(SHADER_DEBUG_FLAGS "")
endif ()

file(GLOB_RECURSE ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/assets/*)

foreach (ASSET IN LISTS ASSETS)
	get_filename_component(FILENAME ${ASSET} NAME)
	get_filename_component(FILE_DIRECTORY ${ASSET} DIRECTORY)
	file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assets ${FILE_DIRECTORY})

	if (NOT REL_PATH STREQUAL "")
		set(REL_PATH ${REL_PATH}/)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/assets/${REL_PATH})
	endif ()

	set(OUT_PATH ${CMAKE_BINARY_DIR}/bin/assets/${REL_PATH}${FILENAME})

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${ASSET} ${OUT_PATH}
			COMMENT " Copying ${ASSET} to ${OUT_PATH}")

	list(APPEND OUT_ASSETS ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME})
	list(APPEND ADDITIONAL_CLEAN_FILES ${OUT_PATH})
endforeach ()

file(GLOB_RECURSE SHADERS
		"${CMAKE_CURRENT_SOURCE_DIR}/shaders/**.frag"
		"${CMAKE_CURRENT_SOURCE_DIR}/shaders/**.vert"
		"${CMAKE_CURRENT_SOURCE_DIR}/shaders/**.tesc"
		"${CMAKE_CURRENT_SOURCE_DIR}/shaders/**.tese"
)

foreach (SHADER IN LISTS SHADERS)
	get_filename_component(FILENAME ${SHADER} NAME)
	get_filename_component(FILE_DIRECTORY ${SHADER} DIRECTORY)
	file(RELATIVE_PATH REL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/shaders ${FILE_DIRECTORY})

	if (NOT REL_PATH STREQUAL "")
		set(REL_PATH ${REL_PATH}/)
		file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/shaders/${REL_PATH})
	endif ()

	set(OUT_PATH ${CMAKE_BINARY_DIR}/bin/shaders/${REL_PATH}${FILENAME}.spv)

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_DEBUG_FLAGS} ${SHADER} -o ${OUT_PATH} DEPENDS ${SHADER}
			COMMENT " Compiling ${SHADER} ")

	list(APPEND SPV_SHADERS ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv)
	list(APPEND ADDITIONAL_CLEAN_FILES ${OUT_PATH})
endforeach ()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_custom_target(assets ALL DEPENDS ${OUT_ASSETS})

add_dependencies(TitorEditor shaders)
add_dependencies(TitorEditor assets)

add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMENT "Copying build commands to src dir")


SetVersionInfo()
CompilerPostSetup()
