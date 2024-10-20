//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include "Control.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/systems/concepts.hpp"

namespace fgl::engine
{
	class Pipeline;

	class CompositionSystem
	{
		std::unique_ptr< Pipeline > m_composite_pipeline { nullptr };

		vk::raii::CommandBuffer& setupSystem( FrameInfo& info );

		CompositionControl m_control {};

	  public:

		CompositionSystem( vk::raii::RenderPass& render_pass );
		~CompositionSystem();

		void pass( FrameInfo& info );
	};

	static_assert( is_system< CompositionSystem > );

} // namespace fgl::engine
