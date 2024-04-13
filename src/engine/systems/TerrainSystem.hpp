//
// Created by kj16609 on 3/11/24.
//

#ifndef GAME_TERRAINSYSTEM_HPP
#define GAME_TERRAINSYSTEM_HPP

#include "concepts.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/buffers/vector/HostVector.hpp"
#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/model/Model.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine
{
	class TerrainSystem
	{
		using VertexShader = VertexShaderT< "shaders/terrain/terrain.vert.spv" >;
		using FragmentShader = FragmentShaderT< "shaders/terrain/terrain.frag.spv" >;
		using TessCShader = TesselationControlShaderT< "shaders/terrain/terrain.tesc.spv" >;
		using TessEShader = TesselationEvaluationShaderT< "shaders/terrain/terrain.tese.spv" >;

		using Shaders = ShaderCollection< VertexShader, FragmentShader, TessCShader, TessEShader >;
		using DescriptorSets = DescriptorSetCollection< GlobalDescriptorSet, TextureDescriptorSet >;

		using Pipeline = PipelineT< Shaders, DescriptorSets >;

		std::unique_ptr< Pipeline > m_pipeline { nullptr };

		std::unique_ptr< Buffer > m_vertex_buffer { nullptr };
		std::unique_ptr< Buffer > m_index_buffer { nullptr };

		using DrawParameterBufferSuballocation = HostVector< vk::DrawIndexedIndirectCommand >;

		using ModelMatrixInfoBufferSuballocation = HostVector< ModelMatrixInfo >;

		std::array< std::unique_ptr< DrawParameterBufferSuballocation >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_draw_parameter_buffers {};

		std::array< std::unique_ptr< ModelMatrixInfoBufferSuballocation >, SwapChain::MAX_FRAMES_IN_FLIGHT >
			m_model_matrix_info_buffers {};

		vk::CommandBuffer& setupSystem( FrameInfo& info );

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

		inline Buffer& getVertexBuffer() { return *m_vertex_buffer; }

		inline Buffer& getIndexBuffer() { return *m_index_buffer; }

		TerrainSystem( Device& device, VkRenderPass render_pass );
		~TerrainSystem() = default;

		void pass( FrameInfo& info );
	};

	static_assert( is_system< TerrainSystem > );
} // namespace fgl::engine

#endif //GAME_TERRAINSYSTEM_HPP
