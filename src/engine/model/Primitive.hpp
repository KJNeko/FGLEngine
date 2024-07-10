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
#include "engine/primitives/TransformComponent.hpp"
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

	struct PrimitiveTextures
	{
		std::shared_ptr< Texture > albedo { nullptr };
		std::shared_ptr< Texture > normal { nullptr };

		bool hasTextures() const { return albedo || normal; }

		bool ready() const
		{
			if ( albedo )
			{
				if ( !albedo->ready() ) return false;
			}

			if ( normal )
			{
				if ( !normal->ready() ) return false;
			}

			return true;
		}
	};

	struct Primitive
	{
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;
		PrimitiveMode m_mode;

		PrimitiveTextures m_textures;

		std::optional< TransformComponent > m_transform;

		//! Returns true if the primitive is ready to be rendered (must have all textures, vertex buffer, and index buffer ready)
		bool ready() const { return m_textures.ready() && m_vertex_buffer.ready() && m_index_buffer.ready(); }

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			const PrimitiveMode mode ) noexcept :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_mode( mode ),
		  m_textures(),
		  m_transform()
		{}

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			PrimitiveTextures&& textures,
			const PrimitiveMode mode ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_mode( mode ),
		  m_textures( std::forward< decltype( m_textures ) >( textures ) ),
		  m_transform()
		{}

		Primitive() = delete;
		Primitive( const Primitive& other ) = delete;
		Primitive( Primitive&& other ) = default;

		static Primitive fromVerts(
			const std::vector< Vertex >&& verts,
			PrimitiveMode mode,
			const std::vector< std::uint32_t >&& indicies,
			memory::Buffer& vertex_buffer,
			memory::Buffer& index_buffer );

		TextureID getAlbedoTextureID() const;
	};

} // namespace fgl::engine
