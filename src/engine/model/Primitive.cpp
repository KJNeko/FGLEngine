//
// Created by kj16609 on 5/23/24.
//

#include "Primitive.hpp"

namespace fgl::engine
{

	Primitive Primitive::fromVerts(
		const std::vector< Vertex >&& verts,
		const PrimitiveMode mode,
		const std::vector< std::uint32_t >&& indicies,
		Buffer& vertex_buffer,
		Buffer& index_buffer )
	{
		const auto bounds { generateBoundingFromVerts( verts ) };
		VertexBufferSuballocation vertex_buffer_suballoc { vertex_buffer, std::move( verts ) };
		IndexBufferSuballocation index_buffer_suballoc { index_buffer, std::move( indicies ) };

		return { std::move( vertex_buffer_suballoc ), std::move( index_buffer_suballoc ), bounds, mode };
	}

	TextureID Primitive::getAlbedoTextureID() const
	{
		if ( m_textures.albedo )
			return m_textures.albedo->getID();
		else
			return std::numeric_limits< TextureID >::max();
	}

} // namespace fgl::engine
