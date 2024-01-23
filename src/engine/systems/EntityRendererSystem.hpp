//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>
#include <vector>

#include "engine/Camera.hpp"
#include "engine/Device.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/GameObject.hpp"
#include "engine/Model.hpp"
#include "engine/SwapChain.hpp"
#include "engine/pipeline/PipelineT.hpp"

namespace fgl::engine
{

	class EntityRendererSystem
	{
		Device& m_device;

		using Pipeline = PipelineT< GlobalDescriptorSet, TextureDescriptorSet >;
		using CompositionPipeline = PipelineT< GBufferDescriptorSet >;

		std::unique_ptr< Pipeline > m_pipeline {};
		std::unique_ptr< CompositionPipeline > m_composition_pipeline {};

		std::unique_ptr< Buffer > m_vertex_buffer { nullptr };
		std::unique_ptr< Buffer > m_index_buffer { nullptr };

		std::array< std::unique_ptr< DrawParameterBufferSuballocation >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_draw_parameter_buffers {};

		std::array< std::unique_ptr< ModelMatrixInfoBufferSuballocation >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_model_matrix_info_buffers {};

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

	  public:

		Buffer& getVertexBuffer() { return *m_vertex_buffer; }

		Buffer& getIndexBuffer() { return *m_index_buffer; }

		void pass( FrameInfo& info );

		EntityRendererSystem( Device& device, VkRenderPass render_pass );
		~EntityRendererSystem();
		EntityRendererSystem( EntityRendererSystem&& other ) = delete;
		EntityRendererSystem( const EntityRendererSystem& other ) = delete;
		EntityRendererSystem& operator=( const EntityRendererSystem& other ) = delete;
	};

} // namespace fgl::engine
