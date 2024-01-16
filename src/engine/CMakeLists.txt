

file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")

set(TRACY_ENABLE "" OFF)
set(TRACY_ON_DEMAND "" ON)

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
)

target_compile_definitions(FGLEngine PUBLIC VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC)

target_link_libraries(FGLEngine PUBLIC Vulkan::Vulkan glfw glm FGLImGui Tracy::TracyClient VMA FGLLoader)
target_include_directories(FGLEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(FGLEngine PROPERTIES COMPILE_FLAGS ${FGL_FLAGS})
#target_compile_definitions(FGLEngine PRIVATE TRACY_ENABLE=1)
target_compile_definitions(FGLEngine PRIVATE ENABLE_IMGUI=1)
