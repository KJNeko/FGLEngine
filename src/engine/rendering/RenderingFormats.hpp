//
// Created by kj16609 on 12/9/24.
//
#pragma once

#include <vulkan/vulkan.hpp>

namespace fgl::engine
{
	vk::Format pickColorFormat();
	vk::Format pickPositionFormat();
	vk::Format pickNormalFormat();
	vk::Format pickDepthFormat();
	vk::Format pickMetallicFormat();
	vk::Format pickEmissiveFormat();
	vk::Format pickCompositeFormat();

} // namespace fgl::engine