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
#include "engine/primitives/boxes/OrientedBoundingBox.hpp"
#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	struct ModelMatrixInfo
	{
		glm::mat4 model_matrix;
		std::uint32_t texture_idx;
	};

	struct ModelBuilder
	{
		Buffer& m_vertex_buffer;
		Buffer& m_index_buffer;

		std::vector< Primitive > m_primitives {};

		ModelBuilder() = delete;

		ModelBuilder( Buffer& parent_vertex_buffer, Buffer& parent_index_buffer ) :
		  m_vertex_buffer( parent_vertex_buffer ),
		  m_index_buffer( parent_index_buffer )
		{}

		void loadModel( const std::filesystem::path& filepath );
		void loadObj( const std::filesystem::path& filepath );
		void loadGltf( const std::filesystem::path& filepath );
	};

	class Model
	{
		Device& m_device;

		static std::vector< vk::DrawIndexedIndirectCommand > buildParameters( const std::vector< Primitive >&
		                                                                          primitives );
		static OrientedBoundingBox< CoordinateSpace::Model > buildBoundingBox( const std::vector< Primitive >&
		                                                                           primitives );

		std::vector< vk::DrawIndexedIndirectCommand > m_draw_parameters;

		std::string m_name { "Unnamed model" };

		//! Bounding box of the model
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;

	  public:

		//! Returns the bounding box in model space
		const OrientedBoundingBox< CoordinateSpace::Model >& getBoundingBox() const { return m_bounding_box; }

		std::vector< Primitive > m_primitives {};

		std::vector< vk::DrawIndexedIndirectCommand > getDrawCommand( const std::uint32_t index ) const;

		static std::unique_ptr< Model > createModel(
			Device& device, const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer );

		void syncBuffers( vk::CommandBuffer& cmd_buffer );

		const std::string& getName() const { return m_name; }

		Model(
			Device& device, ModelBuilder& builder, const OrientedBoundingBox< CoordinateSpace::Model > bounding_box );

		~Model() = default;

		Model( const Model& model ) = delete;
		Model& operator=( const Model& other ) = delete;
		Model( Model&& other ) = delete;
	};

} // namespace fgl::engine