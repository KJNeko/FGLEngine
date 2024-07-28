//
// Created by kj16609 on 7/27/24.
//

#pragma once
#include "engine/descriptors/DescriptorSetLayout.hpp"

namespace fgl::engine
{
	using CameraDescriptor =
		descriptors::Descriptor< 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics >;

	using CameraDescriptorSet = descriptors::DescriptorSetLayout< 1, CameraDescriptor >;
} // namespace fgl::engine