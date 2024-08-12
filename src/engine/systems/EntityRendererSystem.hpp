//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>

#include "engine/buffers/vector/HostVector.hpp"
#include "engine/model/Model.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/systems/modelRendering/StandardPipeline.hpp"
#include "engine/systems/modelRendering/TexturedPipeline.hpp"

namespace fgl::engine
{
	class Device;

	namespace memory
	{
		class Buffer;
	}

	struct FrameInfo;

	class EntityRendererSystem
	{
		Device& m_device;

		//! Standard pipeline for textureless models
		std::unique_ptr< StandardPipeline > m_standard_pipeline {};

		//! Pipeline for basic textured models (Single texture)
		std::unique_ptr< TexturedPipeline > m_textured_pipeline {};

		using DrawParameterBufferSuballocation = HostVector< vk::DrawIndexedIndirectCommand >;

		using ModelMatrixInfoBufferSuballocation = HostVector< ModelMatrixInfo >;

		// Simple parameter buffers
		PerFrameArray< std::unique_ptr< DrawParameterBufferSuballocation > > m_draw_simple_parameter_buffers {};
		PerFrameArray< std::unique_ptr< ModelMatrixInfoBufferSuballocation > > m_simple_model_matrix_info_buffers {};

		// Textured parameter buffers
		PerFrameArray< std::unique_ptr< DrawParameterBufferSuballocation > > m_draw_textured_parameter_buffers {};
		PerFrameArray< std::unique_ptr< ModelMatrixInfoBufferSuballocation > > m_textured_model_matrix_info_buffers {};

		vk::raii::CommandBuffer& setupSystem( FrameInfo& );

	  public:

		void pass( FrameInfo& info );
		void texturelessPass( FrameInfo& info );
		void texturedPass( FrameInfo& info );

		EntityRendererSystem( Device& device, vk::raii::RenderPass& render_pass );
		~EntityRendererSystem() = default;
		EntityRendererSystem( EntityRendererSystem&& other ) = delete;
		EntityRendererSystem( const EntityRendererSystem& other ) = delete;
		EntityRendererSystem& operator=( const EntityRendererSystem& other ) = delete;
	};

} // namespace fgl::engine
