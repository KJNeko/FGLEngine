//
// Created by kj16609 on 8/10/24.
//

#pragma once
#include "engine/descriptors/Descriptor.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"
#include "engine/rendering/SwapChain.hpp"

namespace fgl::engine
{
	class Pipeline;
	struct VertexLine;
	struct FrameInfo;
	struct ModelVertex;

	class LineDrawer
	{
		std::unique_ptr< Pipeline > m_pipeline {};

		PerFrameArray< std::unique_ptr< HostVector< VertexLine > > > m_line_vertex_buffer {};

	  public:

		FGL_DELETE_ALL_RO5( LineDrawer );

		vk::raii::CommandBuffer& setupSystem( FrameInfo& info );
		void pass( FrameInfo& info );

		LineDrawer( vk::raii::RenderPass& render_pass );

		~LineDrawer();
	};

} // namespace fgl::engine