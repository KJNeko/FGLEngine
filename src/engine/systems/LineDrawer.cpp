//
// Created by kj16609 on 8/10/24.
//

#include "LineDrawer.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/buffers/vector/HostVector.hpp"
#include "engine/camera/CameraDescriptor.hpp"
#include "engine/model/ModelVertex.hpp"
#include "engine/model/SimpleVertex.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{

	struct VertexLine
	{
		SimpleVertex p1 {};
		SimpleVertex p2 {};
	};

	inline static std::vector< VertexLine > m_lines {};

	LineDrawer::LineDrawer( Device& device, vk::raii::RenderPass& render_pass )
	{
		PipelineConfigInfo config { render_pass };

		PipelineConfigInfo::addGBufferAttachmentsConfig( config );
		PipelineConfigInfo::setVertexInputType( config, Simple );
		PipelineConfigInfo::setLineTopo( config );

		m_pipeline = std::make_unique< LinePipeline >( device, std::move( config ) );
	}

	LineDrawer::~LineDrawer()
	{}

	vk::raii::CommandBuffer& LineDrawer::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, CameraDescriptorSet::m_set_idx, info.getCameraDescriptor() );

		return command_buffer;
	}

	void LineDrawer::pass( FrameInfo& info )
	{
		ZoneScopedN( "Debug line drawing" );
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Draw debug lines" );

		if ( m_lines.size() == 0 )
		{
			VertexLine line;
			auto& [ p1, p2 ] = line;
			p1.m_position = constants::WORLD_CENTER;
			p2.m_position = constants::WORLD_FORWARD * 20.0f;

			m_lines.emplace_back( line );
		}

		auto& line_vertex_buffer { m_line_vertex_buffer[ info.frame_idx ] };

		line_vertex_buffer = std::make_unique< HostVector< VertexLine > >( info.model_matrix_info_buffer, m_lines );

		command_buffer.bindVertexBuffers( 0, line_vertex_buffer->getVkBuffer(), { line_vertex_buffer->getOffset() } );

		command_buffer.draw( m_lines.size() * 2, m_lines.size(), 0, 0 );
	}

	void cleanupLineQueue()
	{
		m_lines.clear();
	}

	namespace debug
	{
		void drawLine( const WorldCoordinate& p1, const WorldCoordinate& p2 )
		{
			VertexLine line {};
			auto& [ p1v, p2v ] = line;

			p1v.m_position = p1.vec();
			p2v.m_position = p2.vec();

			m_lines.emplace_back( std::move( line ) );
		}
	} // namespace debug

} // namespace fgl::engine