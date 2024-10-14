//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <cstdint>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "ModelVertex.hpp"
#include "engine/memory/buffers/vector/DeviceVector.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "objectloaders/tiny_gltf.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{
	class Material;
	class Texture;

	using VertexBufferSuballocation = DeviceVector< ModelVertex >;

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
		std::shared_ptr< Texture > metallic_roughness { nullptr };

		bool hasTextures() const { return albedo || normal; }

		bool ready() const;
	};

	struct Primitive
	{
		bool draw { true };
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;
		PrimitiveMode m_mode;

		// PrimitiveTextures m_textures;

		std::shared_ptr< Material > m_material;

		std::string m_name { "Unnamed Primitive" };

		//! Returns true if the primitive is ready to be rendered (must have all textures, vertex buffer, and index buffer ready)
		bool ready() const;

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			PrimitiveMode mode ) noexcept;

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			std::shared_ptr< Material >&& material,
			PrimitiveMode mode );

		Primitive() = delete;
		Primitive( const Primitive& other ) = delete;
		Primitive( Primitive&& other ) = default;

		static Primitive fromVerts(
			const std::vector< ModelVertex >&& verts,
			PrimitiveMode mode,
			const std::vector< std::uint32_t >&& indicies,
			memory::Buffer& vertex_buffer,
			memory::Buffer& index_buffer );

		OrientedBoundingBox< CoordinateSpace::Model > getBoundingBox() const;
	};

} // namespace fgl::engine
