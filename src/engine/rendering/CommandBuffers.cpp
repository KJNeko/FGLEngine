//
// Created by kj16609 on 2/14/25.
//

#include "CommandBuffers.hpp"

namespace fgl::engine
{

	CommandBuffers::CommandBuffers( std::vector< CommandBuffer >&& buffers ) :
	  imgui_cb( std::move( buffers[ 0 ] ) ),
	  transfer_cb( std::move( buffers[ 1 ] ) ),
	  render_cb( std::move( buffers[ 2 ] ) ),
	  composition_cb( std::move( buffers[ 3 ] ) )
	{
		imgui_cb.setName( "ImGui CB" );
		transfer_cb.setName( "Transfer CB" );
		render_cb.setName( "Render CB" );
		composition_cb.setName( "Composition CB" );
	}
} // namespace fgl::engine
