//
// Created by kj16609 on 5/23/24.
//

#include "Primitive.hpp"

#include "Model.hpp"
#include "engine/assets/material/Material.hpp"
#include "engine/assets/model/ModelVertex.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "memory/buffers/BufferSuballocationHandle.hpp"

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
		return default_material->ready() && m_vertex_buffer->ready() && m_index_buffer->ready();
	}

	std::shared_ptr< PrimitiveRenderInfoIndex > Primitive::buildRenderInfo()
	{
		auto& buffers { getModelBuffers() };

		PrimitiveRenderInfo info {};
		info.m_first_vert = m_vertex_buffer.getOffsetCount();
		info.m_first_index = m_index_buffer.getOffsetCount();
		info.m_num_indicies = m_index_buffer.size();

		auto ptr { std::make_shared< PrimitiveRenderInfoIndex >( buffers.m_primitive_info.acquire( info ) ) };
		return ptr;
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
	  m_primitive_info( buildRenderInfo() ),
	  default_material()
	{
		assert( m_bounding_box.getTransform().scale != glm::vec3( 0.0f ) );
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
	  m_primitive_info( buildRenderInfo() ),
	  default_material( material )
	{
		assert( m_bounding_box.getTransform().scale != glm::vec3( 0.0f ) );
	}

	Primitive Primitive::fromVerts(
		std::vector< ModelVertex >&& verts,
		const PrimitiveMode mode,
		std::vector< std::uint32_t >&& indicies,
		memory::Buffer& vertex_buffer,
		memory::Buffer& index_buffer )
	{
		const auto bounds { generateBoundingFromVerts( verts ) };
		VertexBufferSuballocation vertex_buffer_suballoc { vertex_buffer,
			                                               std::forward< std::vector< ModelVertex > >( verts ) };

		IndexBufferSuballocation index_buffer_suballoc { index_buffer,
			                                             std::forward< std::vector< std::uint32_t > >( indicies ) };

		return { std::move( vertex_buffer_suballoc ), std::move( index_buffer_suballoc ), bounds, mode };
	}

	OrientedBoundingBox< CoordinateSpace::Model > Primitive::getBoundingBox() const
	{
		return m_bounding_box;
	}

} // namespace fgl::engine
