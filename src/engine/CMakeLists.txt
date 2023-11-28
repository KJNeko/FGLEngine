

file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/**.cpp")

find_package(Vulkan REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

set(TRACY_ENABLE "" OFF)
set(TRACY_ON_DEMAND "" ON)

add_library(FGLEngine STATIC ${CPP_SOURCES})
target_link_libraries(FGLEngine PRIVATE Vulkan::Vulkan glfw glm FGLImGui tracy)
target_include_directories(FGLEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
set_target_properties(FGLEngine PROPERTIES COMPILE_FLAGS ${FGL_FLAGS})
#target_compile_definitions(FGLEngine PRIVATE TRACY_ENABLE="1")

