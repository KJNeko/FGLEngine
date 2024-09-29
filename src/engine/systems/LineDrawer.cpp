//
// Created by kj16609 on 8/10/24.
//

#include "LineDrawer.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/assets/model/ModelVertex.hpp"
#include "engine/assets/model/SimpleVertex.hpp"
#include "engine/camera/CameraDescriptor.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"
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
		debug::drawAxisHelper();

		ZoneScopedN( "Debug line drawing" );
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, *command_buffer, "Draw debug lines" );

		if ( m_lines.size() == 0 ) return;

		auto& line_vertex_buffer { m_line_vertex_buffer[ info.frame_idx ] };

		line_vertex_buffer = std::make_unique< HostVector< VertexLine > >( info.model_matrix_info_buffer, m_lines );

		command_buffer.bindVertexBuffers( 0, line_vertex_buffer->getVkBuffer(), { line_vertex_buffer->getOffset() } );

		command_buffer.setLineWidth( 5.0f );

		command_buffer.draw( m_lines.size() * 2, m_lines.size(), 0, 0 );

		m_lines.clear();
	}

	namespace debug
	{
		void drawLine( const WorldCoordinate& p1, const WorldCoordinate& p2, const glm::vec3 color )
		{
			VertexLine line {};
			auto& [ p1v, p2v ] = line;

			p1v.m_position = p1.vec();
			p1v.m_color = color;
			p2v.m_position = p2.vec();
			p2v.m_color = color;

			m_lines.emplace_back( std::move( line ) );
		}
	} // namespace debug

} // namespace fgl::engine