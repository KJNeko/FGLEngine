find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
	error("Vulkan not found")
endif ()

#add_library(VulkanHppModule)
#
#target_include_directories(VulkanHppModule
#		PRIVATE
#		"${Vulkan_INCLUDE_DIR}"
#)
#
#target_sources(VulkanHppModule PUBLIC
#		FILE_SET CXX_MODULES
#		BASE_DIRS ${Vulkan_INCLUDE_DIR}
#		FILES ${Vulkan_INCLUDE_DIR}/vulkan/vulkan.cppm
#)
#target_compile_definitions(VulkanHppModule PUBLIC VULKAN_HPP_DISPATCH_LOADER_DYNAMIC=1 VULKAN_HPP_NO_STRUCT_CONSTRUCTORS)
#
message("Vulkan include: ${Vulkan_INCLUDE_DIR}")