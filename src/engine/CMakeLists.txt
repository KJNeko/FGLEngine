

file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")

add_library(FGLEngine STATIC ${CPP_SOURCES})
target_precompile_headers(FGLEngine PRIVATE
		<vector>
		<iostream>
		<vulkan/vulkan.h>
		<tracy/Tracy.hpp>
		<vma/vma_impl.hpp>
		<glm/glm.hpp>
		<glm/gtc/matrix_transform.hpp>
		<glm/gtc/constants.hpp>
		<array>
		<chrono>
		<optional>
		<imgui/imgui.h>
)

target_compile_definitions(FGLEngine PUBLIC VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC)

target_link_libraries(FGLEngine PUBLIC Vulkan::Vulkan glfw glm FGLImGui Tracy::TracyClient VMA FGLLoader)
target_include_directories(FGLEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(FGLEngine PROPERTIES COMPILE_FLAGS ${FGL_FLAGS})

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_UPPER_BUILD_TYPE)
if (CMAKE_UPPER_BUILD_TYPE STREQUAL "DEBUG" OR CMAKE_UPPER_BUILD_TYPE STREQUAL "RELWITHDEBINFO")
	target_compile_definitions(FGLEngine PRIVATE ENABLE_IMGUI=1)
	target_compile_definitions(FGLEngine PRIVATE ENABLE_IMGUI_DRAWERS=0)
else ()
	target_compile_definitions(FGLEngine PRIVATE ENABLE_IMGUI=0)
	target_compile_definitions(FGLEngine PRIVATE ENABLE_IMGUI_DRAWERS=0)
endif ()

#GLM settings
target_compile_definitions(FGLEngine PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)
