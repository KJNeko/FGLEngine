//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>

#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/model/Model.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/systems/modelRendering/StandardPipeline.hpp"
#include "engine/systems/modelRendering/TexturedPipeline.hpp"

namespace fgl::engine
{
	class Device;
	class Buffer;
	struct FrameInfo;

	class EntityRendererSystem
	{
		Device& m_device;

		/*
		using VertexShader = VertexShaderT< "shaders/gbuffer.vert.spv" >;
		using FragmentShader = FragmentShaderT< "shaders/gbuffer.frag.spv" >;
		using Shaders = ShaderCollection< VertexShader, FragmentShader >;

		using DescriptorSets = DescriptorSetCollection< GlobalDescriptorSet, TextureDescriptorSet >;

		using Pipeline = PipelineT< Shaders, DescriptorSets >;

		std::unique_ptr< Pipeline > m_pipeline {};
		*/

		//! Standard pipeline for textureless models
		std::unique_ptr< StandardPipeline > m_standard_pipeline {};

		//! Pipeline for basic textured models (Single texture)
		std::unique_ptr< TexturedPipeline > m_textured_pipeline {};

		std::unique_ptr< Buffer > m_vertex_buffer { nullptr };
		std::unique_ptr< Buffer > m_index_buffer { nullptr };

		using DrawParameterBufferSuballocation = HostVector< vk::DrawIndexedIndirectCommand >;

		using ModelMatrixInfoBufferSuballocation = HostVector< ModelMatrixInfo >;

		template < typename T >
		using PerFrameArray = std::array< T, SwapChain::MAX_FRAMES_IN_FLIGHT >;

		// Simple parameter buffers
		PerFrameArray< std::unique_ptr< DrawParameterBufferSuballocation > > m_draw_simple_parameter_buffers {};
		PerFrameArray< std::unique_ptr< ModelMatrixInfoBufferSuballocation > > m_simple_model_matrix_info_buffers {};

		// Textured parameter buffers
		PerFrameArray< std::unique_ptr< DrawParameterBufferSuballocation > > m_draw_textured_parameter_buffers {};
		PerFrameArray< std::unique_ptr< ModelMatrixInfoBufferSuballocation > > m_textured_model_matrix_info_buffers {};

		void initVertexBuffer( std::uint32_t size )
		{
			m_vertex_buffer = std::make_unique< Buffer >(
				size,
				vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal );
		}

		void initIndexBuffer( std::uint32_t size )
		{
			m_index_buffer = std::make_unique< Buffer >(
				size,
				vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eDeviceLocal );
		}

		vk::CommandBuffer& setupSystem( FrameInfo& );

	  public:

		Buffer& getVertexBuffer() { return *m_vertex_buffer; }

		Buffer& getIndexBuffer() { return *m_index_buffer; }

		void pass( FrameInfo& info );
		void texturelessPass( FrameInfo& info );
		void texturedPass( FrameInfo& info );

		EntityRendererSystem( Device& device, VkRenderPass render_pass );
		~EntityRendererSystem() = default;
		EntityRendererSystem( EntityRendererSystem&& other ) = delete;
		EntityRendererSystem( const EntityRendererSystem& other ) = delete;
		EntityRendererSystem& operator=( const EntityRendererSystem& other ) = delete;
	};

} // namespace fgl::engine
