

file(GLOB_RECURSE CPP_SOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/**.cpp"
		"${CMAKE_CURRENT_SOURCE_DIR}/**.hpp"
)

add_library(FGLEngine STATIC ${CPP_SOURCES} ${HPP_SOURCES})
set(CMAKE_CXX_STANDARD 23)

if (DEFINED FGL_ENABLE_PCH)

	target_precompile_headers(FGLEngine PRIVATE

			<vulkan/vulkan.h>

			<tracy/Tracy.hpp>

			<vma/vma_impl.hpp>

			<glm/glm.hpp>
			<glm/gtc/matrix_transform.hpp>
			<glm/gtc/constants.hpp>
			<glm/vec3.hpp>
			<glm/vec2.hpp>
			<glm/mat4x4.hpp>
	)

endif ()

target_compile_definitions(FGLEngine PUBLIC VULKAN_HPP_FLAGS_MASK_TYPE_AS_PUBLIC)

include(dependencies/spdlog)
include(dependencies/imgui)

target_link_libraries(FGLEngine PUBLIC Vulkan::Vulkan glfw glm ImGui Tracy::TracyClient VMA FGLLoader spdlog)
target_include_directories(FGLEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(FGLEngine PROPERTIES COMPILE_FLAGS ${FGL_FLAGS})
target_compile_features(FGLEngine PRIVATE cxx_std_23)

string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_UPPER_BUILD_TYPE)

if (NOT DEFINED FGL_ENABLE_IMGUI AND CMAKE_UPPER_BUILD_TYPE STREQUAL "DEBUG")
	set(FGL_ENABLE_IMGUI 1)
endif ()

message("-- FGL_ENABLE_IMGUI: ${FGL_ENABLE_IMGUI}")

if (FGL_ENABLE_IMGUI)
	target_compile_definitions(FGLEngine PUBLIC ENABLE_IMGUI=1)
	target_compile_definitions(FGLEngine PUBLIC ENABLE_IMGUI_DRAWERS=1)
else ()
	target_compile_definitions(FGLEngine PUBLIC ENABLE_IMGUI=0)
	target_compile_definitions(FGLEngine PUBLIC ENABLE_IMGUI_DRAWERS=0)
endif ()

if (NOT DEFINED FGL_ENABLE_TESTS)
	set(FGL_ENABLE_TESTS 0)
endif ()

if (FGL_ENABLE_TESTS)
	target_compile_definitions(FGLEngine PUBLIC FGL_TESTS=1)
	target_link_libraries(FGLEngine PUBLIC Catch2)
else ()
	target_compile_definitions(FGLEngine PUBLIC FGL_TESTS=0)
endif ()


#GLM settings
# GLM_FORCE_NO_CTOR_INIT
target_compile_definitions(FGLEngine PUBLIC GLM_FORCE_RADIANS GLM_FORCE_DEPTH_ZERO_TO_ONE GLM_FORCE_LEFT_HANDED)
