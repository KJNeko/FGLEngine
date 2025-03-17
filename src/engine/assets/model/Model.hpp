//
// Created by kj16609 on 11/28/23.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include <filesystem>
#include <memory>
#include <vector>

#include "Primitive.hpp"
#include "assets/AssetManager.hpp"
#include "engine/assets/material/Material.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "memory/buffers/vector/IndexedVector.hpp"

namespace fgl::engine
{
	struct ModelRenderInfo;
	class ModelRenderHandle;

	namespace memory
	{
		class Buffer;
	}

	struct ModelBuilder;

	struct ModelMatrixInfo
	{
		glm::mat4 model_matrix;
		MaterialID material_id { constants::INVALID_TEXTURE_ID };
	};

	class Model final : public AssetInterface< Model >
	{
		static OrientedBoundingBox< CoordinateSpace::Model >
			buildBoundingBox( const std::vector< Primitive >& primitives );

		std::string m_name { "Unnamed model" };

		//! Bounding box of the model
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;

		IndexedVector< ModelMatrixInfo >::Index m_model_info {};

	  public:

		[[nodiscard]] bool ready() const;

		//! Returns the bounding box in model space
		[[nodiscard]] const OrientedBoundingBox< CoordinateSpace::Model >& getBoundingBox() const
		{
			return m_bounding_box;
		}

		std::vector< Primitive > m_primitives {};

		[[nodiscard]] std::vector< vk::DrawIndexedIndirectCommand > getDrawCommand( std::uint32_t index ) const;

		//TODO: Switch to using shared_ptr instead of unique_ptr
		static std::shared_ptr< Model > createModel(
			const std::filesystem::path& path, memory::Buffer& vertex_buffer, memory::Buffer& index_buffer );

		static std::shared_ptr< Model > createModelFromVerts(
			std::vector< ModelVertex > verts,
			std::vector< std::uint32_t > indicies,
			memory::Buffer& vertex_buffer,
			memory::Buffer& index_buffer );

		const std::string& getName() const { return m_name; }

		Model(
			ModelBuilder& builder,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			const std::string& name = {} );

		Model(
			std::vector< Primitive >&& primitives,
			const OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			const std::string& name = {} );

		~Model() = default;

		Model( const Model& model ) = delete;
		Model& operator=( const Model& other ) = delete;
		Model( Model&& other ) = delete;
	};

} // namespace fgl::engine