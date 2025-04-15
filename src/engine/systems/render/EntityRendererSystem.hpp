//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <memory>

#include "engine/rendering/PresentSwapChain.hpp"

namespace fgl::engine
{
	class Pipeline;
	class Device;

	namespace memory
	{
		class BufferHandle;
	}

	struct FrameInfo;

	class EntityRendererSystem
	{
		//! Standard pipeline for textureless models
		std::unique_ptr< Pipeline > m_standard_pipeline;

		//! Pipeline for basic textured models (Single texture)
		std::unique_ptr< Pipeline > m_textured_pipeline;
		// std::unique_ptr< ComputePipeline > m_cull_pipeline {};

		CommandBuffer& setupSystem( const FrameInfo& );

	  public:

		void pass( FrameInfo& info );
		void texturedPass( const FrameInfo& info );

		EntityRendererSystem();
		~EntityRendererSystem();
		EntityRendererSystem( EntityRendererSystem&& other ) = delete;
		EntityRendererSystem( const EntityRendererSystem& other ) = delete;
		EntityRendererSystem& operator=( const EntityRendererSystem& other ) = delete;
	};

} // namespace fgl::engine
