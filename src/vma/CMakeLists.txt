


add_library(VMA STATIC ${CMAKE_CURRENT_SOURCE_DIR}/vma_impl.cpp)
target_link_libraries(VMA PUBLIC Vulkan::Vulkan GPUOpen::VulkanMemoryAllocator)
