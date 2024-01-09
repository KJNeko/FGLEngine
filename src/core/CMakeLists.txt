

add_executable(${PROJECT_NAME} "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(${PROJECT_NAME} PRIVATE FGLEngine Vulkan::Vulkan glfw glm FGLImGui Tracy::TracyClient GPUOpen::VulkanMemoryAllocator)
