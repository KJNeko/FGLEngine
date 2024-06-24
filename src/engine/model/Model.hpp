//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <glm/glm.hpp>

#include <filesystem>
#include <memory>
#include <vector>

#include "Primitive.hpp"
#include "engine/buffers/Buffer.hpp"
#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{
	struct ModelBuilder;

	struct ModelMatrixInfo
	{
		glm::mat4 model_matrix;
		std::uint32_t texture_idx;
	};

	class Model
	{
		static std::vector< vk::DrawIndexedIndirectCommand > buildParameters( const std::vector< Primitive >&
		                                                                          primitives );
		static OrientedBoundingBox< CoordinateSpace::Model > buildBoundingBox( const std::vector< Primitive >&
		                                                                           primitives );

		TransformComponent m_model_transform;

		std::vector< vk::DrawIndexedIndirectCommand > m_draw_parameters;

		std::string m_name { "Unnamed model" };

		//! Bounding box of the model
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;

	  public:

		//! Returns the bounding box in model space
		const OrientedBoundingBox< CoordinateSpace::Model >& getBoundingBox() const { return m_bounding_box; }

		std::vector< Primitive > m_primitives {};

		std::vector< vk::DrawIndexedIndirectCommand > getDrawCommand( const std::uint32_t index ) const;

		//TODO: Switch to using shared_ptr instead of unique_ptr
		static std::shared_ptr< Model >
			createModel( const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer );

		static std::shared_ptr< Model > createModelFromVerts(
			std::vector< Vertex > verts,
			std::vector< std::uint32_t > indicies,
			Buffer& vertex_buffer,
			Buffer& index_buffer );

		static std::vector< std::shared_ptr< Model > >
			createModelsFromScene( const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer );

		void stage( vk::raii::CommandBuffer& cmd_buffer );

		const std::string& getName() const { return m_name; }

		Model(
			ModelBuilder& builder,
			const OrientedBoundingBox< CoordinateSpace::Model > bounding_box,
			std::string name = {} );

		Model(
			std::vector< Primitive >&& primitives,
			const OrientedBoundingBox< CoordinateSpace::Model > bounding_box,
			std::string name = {} );

		~Model() = default;

		Model( const Model& model ) = delete;
		Model& operator=( const Model& other ) = delete;
		Model( Model&& other ) = delete;
	};

} // namespace fgl::engine