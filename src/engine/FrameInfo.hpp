//
// Created by kj16609 on 11/30/23.
//

#pragma once

//clang-format: off
#include <tracy/TracyVulkan.hpp>
#include <vulkan/vulkan.hpp>
//clang-format: on

#include "descriptors/Descriptor.hpp"
#include "descriptors/DescriptorSetLayout.hpp"
#include "primitives/Frustum.hpp"
#include "rendering/types.hpp"

#define MAX_LIGHTS 10

namespace fgl::engine
{
	class Pipeline;
	class GameObject;

	namespace descriptors
	{
		class DescriptorSet;
	}

	class PresentSwapChain;
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

	constexpr descriptors::Descriptor texture_descriptor { 0,
		                                                   vk::DescriptorType::eCombinedImageSampler,
		                                                   vk::ShaderStageFlagBits::eAllGraphics,
		                                                   512,
		                                                   vk::DescriptorBindingFlagBits::eUpdateAfterBind
		                                                       | vk::DescriptorBindingFlagBits::ePartiallyBound };

	constexpr std::uint32_t TEXTURE_SET_ID { 2 };

	constexpr vk::ShaderStageFlags FRAG_STAGE { vk::ShaderStageFlagBits::eFragment };

	constexpr descriptors::AttachmentDescriptor color_descriptor { 0, FRAG_STAGE };
	constexpr descriptors::AttachmentDescriptor position_descriptor { 1, FRAG_STAGE };
	constexpr descriptors::AttachmentDescriptor normal_descriptor { 2, FRAG_STAGE };
	constexpr descriptors::AttachmentDescriptor metallic_descriptor { 3, FRAG_STAGE };
	constexpr descriptors::AttachmentDescriptor emissive_descriptor { 4, FRAG_STAGE };

	//TODO: Move this from being static, It being here prevents safe cleanup
	inline static descriptors::DescriptorSetLayout gbuffer_set {
		0, color_descriptor, position_descriptor, normal_descriptor, metallic_descriptor, emissive_descriptor
	};

	constexpr descriptors::AttachmentDescriptor input_descriptor { 0, vk::ShaderStageFlagBits::eFragment };

	inline static descriptors::DescriptorSetLayout gui_descriptor_set { 0, input_descriptor };

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

		// descriptors::DescriptorSet& gui_input_descriptor;

		descriptors::DescriptorSet& getGBufferDescriptor() const;
		descriptors::DescriptorSet& getCameraDescriptor() const;

		PresentSwapChain& swap_chain;

		std::vector< std::vector< GameObject >* > in_view_leafs {};

		//! Binds the camera descriptor to the command buffer
		void bindCamera( Pipeline& pipeline );
	};

} // namespace fgl::engine
