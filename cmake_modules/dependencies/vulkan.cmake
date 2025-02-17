find_package(Vulkan REQUIRED)
if (NOT Vulkan_FOUND)
	error("Vulkan not found")
endif ()

message("Vulkan include: ${Vulkan_INCLUDE_DIR}")