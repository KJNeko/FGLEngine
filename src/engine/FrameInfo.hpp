//
// Created by kj16609 on 11/30/23.
//

#pragma once

#include <vulkan/vulkan.h>

//clang-format: off
#include <tracy/TracyVulkan.hpp>
//clang-format: on

#include "Camera.hpp"
#include "GameObject.hpp"
#include "PushConstant.hpp"
#include "engine/descriptors/Descriptor.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

#define MAX_LIGHTS 10

namespace fgl::engine
{
	class SwapChain;

	struct PointLight
	{
		glm::vec4 position {}; //ignore w
		glm::vec4 color {};
	};

	struct CameraInfo
	{
		glm::mat4 projection { 1.0f };
		glm::mat4 view { 1.0f };
		glm::mat4 inverse_view { 1.0f };
	};

	struct PointLightUBO
	{
		alignas( 16 ) PointLight point_lights[ MAX_LIGHTS ] {};
		alignas( 16 ) int num_lights { 0 };
	};

	using CameraDescriptor = Descriptor< 0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics >;
	using LightDescriptor = Descriptor< 2, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics >;

	using GlobalDescriptorSet = DescriptorSetLayout< 0, CameraDescriptor, EmptyDescriptor< 1 >, LightDescriptor >;

	using TextureDescriptor = Descriptor<
		0,
		vk::DescriptorType::eCombinedImageSampler,
		vk::ShaderStageFlagBits::eAllGraphics,
		512,
		vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound >;

	using TextureDescriptorSet = DescriptorSetLayout< 1, TextureDescriptor >;

	using PositionDescriptor = AttachmentDescriptor< 0, vk::ShaderStageFlagBits::eFragment >;
	using NormalDescriptor = AttachmentDescriptor< 1, vk::ShaderStageFlagBits::eFragment >;
	using AlbedoDescriptor = AttachmentDescriptor< 2, vk::ShaderStageFlagBits::eFragment >;

	static_assert( is_descriptor< PositionDescriptor > );

	using GBufferDescriptorSet = DescriptorSetLayout< 0, PositionDescriptor, NormalDescriptor, AlbedoDescriptor >;

	using CompositeDescriptor = AttachmentDescriptor< 0, vk::ShaderStageFlagBits::eFragment >;

	using GBufferCompositeDescriptorSet = DescriptorSetLayout< 0, CompositeDescriptor >;

	class OctTreeNode;

	struct FrameInfo
	{
		std::uint16_t frame_idx;
		float frame_time;

		vk::raii::CommandBuffer& command_buffer;
		vk::raii::CommandBuffer& gui_command_buffer;

		struct
		{
			Camera& camera;
			TransformComponent& camera_transform;
		} camera_data;

		DescriptorSet& global_descriptor_set;
		OctTreeNode& game_objects;
		TracyVkCtx tracy_ctx;

		//Buffers
		Buffer& model_matrix_info_buffer;
		Buffer& draw_parameter_buffer;

		DescriptorSet& gbuffer_descriptor_set;
		DescriptorSet& gbuffer_composite_set;

		const Frustum< CoordinateSpace::World >& camera_frustum;
		SwapChain& swap_chain;

		std::vector< std::vector< GameObject >* > in_view_leafs {};
	};

} // namespace fgl::engine
