//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <cstdint>

#include "Vertex.hpp"
#include "engine/buffers/vector/DeviceVector.hpp"
#include "engine/texture/Texture.hpp"

namespace fgl::engine
{

	using VertexBufferSuballocation = DeviceVector< Vertex >;

	using IndexBufferSuballocation = DeviceVector< std::uint32_t >;

	struct Primitive
	{
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;

		std::optional< Texture > m_texture { std::nullopt };

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			OrientedBoundingBox< CoordinateSpace::Model >& bounding_box ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box )
		{}

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			Texture&& texture ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_texture( std::move( texture ) )
		{}

		Primitive() = delete;
		Primitive( const Primitive& other ) = delete;
		Primitive( Primitive&& other ) = default;
	};

} // namespace fgl::engine
