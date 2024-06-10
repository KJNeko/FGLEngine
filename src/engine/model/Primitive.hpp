//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <cstdint>

#include "Vertex.hpp"
#include "engine/buffers/vector/DeviceVector.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/texture/Texture.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "objectloaders/tiny_gltf.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{
	using VertexBufferSuballocation = DeviceVector< Vertex >;

	using IndexBufferSuballocation = DeviceVector< std::uint32_t >;

	enum PrimitiveMode
	{
		POINTS = TINYGLTF_MODE_POINTS,
		LINE = TINYGLTF_MODE_LINE,
		LINE_LOOP = TINYGLTF_MODE_LINE_LOOP,
		LINE_STRIP = TINYGLTF_MODE_LINE_STRIP,
		TRIS = TINYGLTF_MODE_TRIANGLES,
		TRI_STRIP = TINYGLTF_MODE_TRIANGLE_STRIP,
		TRI_FAN = TINYGLTF_MODE_TRIANGLE_FAN
	};

	struct Primitive
	{
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;
		PrimitiveMode m_mode;

		std::shared_ptr< Texture > m_texture;

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			const PrimitiveMode mode ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_mode( mode )
		{}

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			std::shared_ptr< Texture >&& texture,
			const PrimitiveMode mode ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_mode( mode ),
		  m_texture( std::forward< decltype( m_texture ) >( texture ) )
		{}

		Primitive() = delete;
		Primitive( const Primitive& other ) = delete;
		Primitive( Primitive&& other ) = default;

		static Primitive fromVerts(
			const std::vector< Vertex >&& verts,
			const PrimitiveMode mode,
			const std::vector< std::uint32_t >&& indicies,
			Buffer& vertex_buffer,
			Buffer& index_buffer );

		TextureID getTextureID() const;
	};

} // namespace fgl::engine
