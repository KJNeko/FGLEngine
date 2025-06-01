//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <cstdint>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "objectloaders/tiny_gltf.h"
#pragma GCC diagnostic pop

#include "ModelInstanceInfo.hpp"
#include "assets/material/Material.hpp"
#include "engine/memory/buffers/vector/DeviceVector.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "memory/buffers/vector/IndexedVector.hpp"

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

	struct PrimitiveRenderInfo
	{
		//! First vertex in the buffer
		std::uint32_t m_first_vert;
		//! First index
		std::uint32_t m_first_index;
		//! Number of indicies
		std::uint32_t m_num_indicies;
	};

	using PrimitiveRenderInfoIndex = IndexedVector< PrimitiveRenderInfo >::Index;

	struct PrimitiveInstanceInfo
	{
		PrimitiveRenderInfoIndex::GPUValue m_primitive_info;
		ModelInstanceInfoIndex::GPUValue m_model_info;
		MaterialID m_material;
	};

	using PrimitiveInstanceInfoIndex = IndexedVector< PrimitiveInstanceInfo >::Index;

	struct Primitive
	{
		bool draw { true };
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;
		PrimitiveMode m_mode;

		std::shared_ptr< PrimitiveRenderInfoIndex > m_primitive_info;

		std::shared_ptr< Material > default_material;

		std::string m_name { "Unnamed Primitive" };

		//! Returns true if the primitive is ready to be rendered (must have all textures, vertex buffer, and index buffer ready)
		[[nodiscard]] bool ready() const;

		std::shared_ptr< PrimitiveRenderInfoIndex > buildRenderInfo();

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

		[[nodiscard]] std::shared_ptr< PrimitiveRenderInfoIndex > renderInstanceInfo() const
		{
			return m_primitive_info;
		}

		static Primitive fromVerts(
			std::vector< ModelVertex >&& verts,
			PrimitiveMode mode,
			std::vector< std::uint32_t >&& indicies,
			memory::Buffer& vertex_buffer,
			memory::Buffer& index_buffer );

		[[nodiscard]] OrientedBoundingBox< CoordinateSpace::Model > getBoundingBox() const;
	};

} // namespace fgl::engine
