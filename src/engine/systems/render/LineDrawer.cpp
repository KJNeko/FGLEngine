//
// Created by kj16609 on 8/10/24.
//

#include "LineDrawer.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/assets/model/SimpleVertex.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"
#include "engine/rendering/pipelines/v2/PipelineBuilder.hpp"

namespace fgl::engine
{

	struct VertexLine
	{
		SimpleVertex p1 {};
		SimpleVertex p2 {};
	};

	inline static std::vector< VertexLine > m_lines {};

	LineDrawer::LineDrawer()
	{
		PipelineBuilder builder { 0 };

		builder.addDescriptorSet( Camera::getDescriptorLayout() );

		addGBufferAttachments( builder );

		builder.setAttributeDescriptions( SimpleVertex::getAttributeDescriptions() );
		builder.setBindingDescriptions( SimpleVertex::getBindingDescriptions() );

		builder.setTopology( vk::PrimitiveTopology::eLineList );

		builder.setVertexShader( Shader::loadVertex( "shaders/line.slang" ) );
		builder.setFragmentShader( Shader::loadFragment( "shaders/line.slang" ) );

		builder.addDynamicState( vk::DynamicState::eLineWidth );

		m_pipeline = builder.create();
	}

	LineDrawer::~LineDrawer()
	{}

	CommandBuffer& LineDrawer::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer.render_cb };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, info.getCameraDescriptor() );

		return command_buffer;
	}

	void LineDrawer::pass( FrameInfo& info )
	{
		/*
		ZoneScopedN( "LineDrawer::pass" );
		debug::drawAxisHelper();

		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, **command_buffer, "Draw debug lines" );

		if ( m_lines.empty() ) return;

		auto& line_vertex_buffer { m_line_vertex_buffer[ info.frame_idx ] };

		line_vertex_buffer = std::make_unique< HostVector< VertexLine > >( info.model_matrix_info_buffer, m_lines );

		command_buffer->bindVertexBuffers( 0, line_vertex_buffer->getVkBuffer(), { line_vertex_buffer->getOffset() } );

		command_buffer->setLineWidth( 5.0f );

		command_buffer->draw(
			static_cast< std::uint32_t >( m_lines.size() * 2 ), static_cast< std::uint32_t >( m_lines.size() ), 0, 0 );

		*/
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