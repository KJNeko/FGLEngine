//
// Created by kj16609 on 10/20/24.
//

#pragma once

#include <vulkan/vulkan_core.h>

#include <vector>

// This file contains all the various extensions we wish to use

inline const static std::vector< const char* > DEVICE_EXTENSIONS = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME, // Used for descriptor indexing

	VK_EXT_MESH_SHADER_EXTENSION_NAME // MAGICAL SHIT
};
