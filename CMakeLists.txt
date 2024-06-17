# /CMakeLists.txt

cmake_minimum_required(VERSION 3.25.0)
project(Game LANGUAGES CXX C)

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
add_subdirectory(tests)

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

	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${ASSET} ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME}
			COMMENT " Copying ${ASSET} ")
	list(APPEND OUT_ASSETS ${CMAKE_BINARY_DIR}/bin/assets/${FILENAME})
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
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_DEBUG_FLAGS} ${SHADER} -o ${CMAKE_BINARY_DIR}/bin/shaders/${REL_PATH}${FILENAME}.spv DEPENDS ${SHADER}
			COMMENT " Compiling ${SHADER} ")
	list(APPEND SPV_SHADERS ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv)
endforeach ()


add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_custom_target(assets ALL DEPENDS ${OUT_ASSETS})

add_dependencies(${PROJECT_NAME} shaders)
add_dependencies(${PROJECT_NAME} assets)


add_custom_command(
		OUTPUT ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/compile_commands.json ${CMAKE_SOURCE_DIR}/compile_commands.json
		COMMENT "Copying build commands to src dir")


SetVersionInfo()
CompilerPostSetup()
