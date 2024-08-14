//
// Created by kj16609 on 11/30/23.
//

#pragma once

#include <vulkan/vulkan.h>

//clang-format: off
#include <tracy/TracyVulkan.hpp>
//clang-format: on

#include "descriptors/Descriptor.hpp"
#include "descriptors/DescriptorSetLayout.hpp"
#include "primitives/Frustum.hpp"
#include "rendering/pipelines/Pipeline.hpp"
#include "rendering/types.hpp"

#define MAX_LIGHTS 10

namespace fgl::engine
{
	class GameObject;

	namespace descriptors
	{
		class DescriptorSet;
	}

	class SwapChain;
	class Camera;

	struct PointLight
	{
		glm::vec4 position {}; //ignore w
		glm::vec4 color {};
	};

	struct PointLightUBO
	{
		alignas( 16 ) PointLight point_lights[ MAX_LIGHTS ] {};
		alignas( 16 ) int num_lights { 0 };
	};

	//using GlobalDescriptorSet = descriptors::DescriptorSetLayout< 0, descriptors::EmptyDescriptor< 0 > >;

	using TextureDescriptor = descriptors::Descriptor<
		0,
		vk::DescriptorType::eCombinedImageSampler,
		vk::ShaderStageFlagBits::eAllGraphics,
		512,
		vk::DescriptorBindingFlagBits::eUpdateAfterBind | vk::DescriptorBindingFlagBits::ePartiallyBound >;

	using TextureDescriptorSet = descriptors::DescriptorSetLayout< 2, TextureDescriptor >;

	using PositionDescriptor = descriptors::AttachmentDescriptor< 0, vk::ShaderStageFlagBits::eFragment >;
	using NormalDescriptor = descriptors::AttachmentDescriptor< 1, vk::ShaderStageFlagBits::eFragment >;
	using AlbedoDescriptor = descriptors::AttachmentDescriptor< 2, vk::ShaderStageFlagBits::eFragment >;

	static_assert( is_descriptor< PositionDescriptor > );

	using GBufferDescriptorSet =
		descriptors::DescriptorSetLayout< 0, PositionDescriptor, NormalDescriptor, AlbedoDescriptor >;

	using InputDescriptor = descriptors::AttachmentDescriptor< 0, vk::ShaderStageFlagBits::eFragment >;

	using GuiInputDescriptorSet = descriptors::DescriptorSetLayout< 0, InputDescriptor >;

	class OctTreeNode;

	struct FrameInfo
	{
		FrameIndex frame_idx;
		PresentIndex present_idx;
		double delta_time;

		vk::raii::CommandBuffer& command_buffer;

		Camera* camera { nullptr };

		std::vector< std::weak_ptr< Camera > >& m_camera_list;

		// descriptors::DescriptorSet& global_descriptor_set;
		OctTreeNode& game_objects;
		TracyVkCtx tracy_ctx;

		//Buffers
		memory::Buffer& model_matrix_info_buffer;
		memory::Buffer& draw_parameter_buffer;

		memory::Buffer& model_vertex_buffer;
		memory::Buffer& model_index_buffer;

		descriptors::DescriptorSet& gui_input_descriptor;

		descriptors::DescriptorSet& getGBufferDescriptor() const;
		descriptors::DescriptorSet& getCameraDescriptor() const;

		SwapChain& swap_chain;

		std::vector< std::vector< GameObject >* > in_view_leafs {};

		//! Binds the camera descriptor to the command buffer
		void bindCamera( internal::Pipeline& pipeline );
	};

} // namespace fgl::engine
