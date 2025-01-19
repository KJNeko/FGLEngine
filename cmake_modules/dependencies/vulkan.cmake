find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
	error("Vulkan not found")
endif ()