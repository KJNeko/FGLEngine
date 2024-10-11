//
// Created by kj16609 on 7/27/24.
//

#pragma once
#include "engine/descriptors/DescriptorSetLayout.hpp"

namespace fgl::engine
{
	constexpr descriptors::Descriptor camera_descriptor { 0,
		                                                  vk::DescriptorType::eUniformBuffer,
		                                                  vk::ShaderStageFlagBits::eAllGraphics };

	inline static descriptors::DescriptorSetLayout camera_descriptor_set { 1, camera_descriptor };

} // namespace fgl::engine