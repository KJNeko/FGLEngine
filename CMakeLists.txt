# /CMakeLists.txt

cmake_minimum_required(VERSION 3.26.4)
project(Game LANGUAGES CXX C)
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/appicon.rc")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
include(dependencies/tracy)
include(dependencies/vulkan)

add_subdirectory(src)

file(GLOB FRAG_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag")
file(GLOB VERT_SHADERS "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert")

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/shaders")
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/models")

file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/models" DESTINATION "${CMAKE_BINARY_DIR}/bin")

foreach (MODEL IN LISTS MODELS)
	get_filename_component(FILENAME ${MODEL} NAME)
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/models/${FILENAME}
			COMMAND ${CMAKE_COMMAND} -E copy ${MODEL} ${CMAKE_BINARY_DIR}/bin/models/${FILENAME}
			COMMENT " Copying ${MODEL} ")
	list(APPEND OBJ_MODELS ${CMAKE_BINARY_DIR}/bin/models/${FILENAME})
endforeach ()

foreach (SHADER IN LISTS FRAG_SHADERS)
	get_filename_component(FILENAME ${SHADER} NAME)
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv DEPENDS ${SHADER}
			COMMENT " Compiling ${SHADER} ")
	list(APPEND SPV_SHADERS ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv)
endforeach ()

foreach (SHADER IN LISTS VERT_SHADERS)
	get_filename_component(FILENAME ${SHADER} NAME)
	add_custom_command(OUTPUT ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv
			COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv DEPENDS ${SHADER}
			COMMENT " Compiling ${SHADER} ")
	list(APPEND SPV_SHADERS ${CMAKE_BINARY_DIR}/bin/shaders/${FILENAME}.spv)
endforeach ()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})
add_custom_target(models ALL DEPENDS ${OBJ_MODELS})

add_dependencies(${PROJECT_NAME} shaders)
add_dependencies(${PROJECT_NAME} models)

SetVersionInfo()
CompilerPostSetup()