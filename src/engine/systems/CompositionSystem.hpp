//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include "concepts.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/rendering/pipelines/Shader.hpp"

namespace fgl::engine
{
		class Pipeline;

	class CompositionSystem
	{

		std::unique_ptr< Pipeline > m_composite_pipeline { nullptr };

		vk::raii::CommandBuffer& setupSystem( FrameInfo& info );

	  public:

		CompositionSystem(  vk::raii::RenderPass& render_pass );
		~CompositionSystem() = default;

		void pass( FrameInfo& info );
	};

	static_assert( is_system< CompositionSystem > );

} // namespace fgl::engine
