//
// Created by kj16609 on 5/23/24.
//

#include "Primitive.hpp"

#include "engine/assets/material/Material.hpp"
#include "engine/assets/texture/Texture.hpp"

namespace fgl::engine
{

	bool PrimitiveTextures::ready() const
	{
		if ( albedo )
		{
			if ( !albedo->ready() ) return false;
		}

		if ( normal )
		{
			if ( !normal->ready() ) return false;
		}

		if ( metallic_roughness )
		{
			if ( !metallic_roughness->ready() ) return false;
		}

		return true;
	}

	bool Primitive::ready() const
	{
		return m_material->ready() && m_vertex_buffer.ready() && m_index_buffer.ready();
	}

	Primitive::Primitive(
		VertexBufferSuballocation&& vertex_buffer,
		IndexBufferSuballocation&& index_buffer,
		const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
		const PrimitiveMode mode ) noexcept :
	  m_vertex_buffer( std::move( vertex_buffer ) ),
	  m_index_buffer( std::move( index_buffer ) ),
	  m_bounding_box( bounding_box ),
	  m_mode( mode ),
	  m_material()
	{
		assert( m_bounding_box.m_transform.scale != glm::vec3( 0.0f ) );
	}

	Primitive::Primitive(
		VertexBufferSuballocation&& vertex_buffer,
		IndexBufferSuballocation&& index_buffer,
		const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
		std::shared_ptr< Material >&& material,
		const PrimitiveMode mode ) :
	  m_vertex_buffer( std::move( vertex_buffer ) ),
	  m_index_buffer( std::move( index_buffer ) ),
	  m_bounding_box( bounding_box ),
	  m_mode( mode ),
	  m_material( material )
	{
		assert( m_bounding_box.m_transform.scale != glm::vec3( 0.0f ) );
	}

	Primitive Primitive::fromVerts(
		const std::vector< ModelVertex >&& verts,
		const PrimitiveMode mode,
		const std::vector< std::uint32_t >&& indicies,
		memory::Buffer& vertex_buffer,
		memory::Buffer& index_buffer )
	{
		const auto bounds { generateBoundingFromVerts( verts ) };
		VertexBufferSuballocation vertex_buffer_suballoc { vertex_buffer, std::move( verts ) };
		IndexBufferSuballocation index_buffer_suballoc { index_buffer, std::move( indicies ) };

		return { std::move( vertex_buffer_suballoc ), std::move( index_buffer_suballoc ), bounds, mode };
	}

	OrientedBoundingBox< CoordinateSpace::Model > Primitive::getBoundingBox() const
	{
		return m_bounding_box;
	}

} // namespace fgl::engine
