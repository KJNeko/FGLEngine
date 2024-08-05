#set(SPIRV-Headers_SOURCE_DIR ${CMAKE_SOURCE_DIR}/dependencies/SPIRV-Headers)

add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/SPIRV-Headers)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/SPIRV-Tools)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/glslang)

set(SHADERC_SKIP_TESTS ON)
add_subdirectory(${CMAKE_SOURCE_DIR}/dependencies/shaderc)