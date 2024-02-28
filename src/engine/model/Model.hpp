//
// Created by kj16609 on 11/28/23.
//

#pragma once

#include <glm/glm.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "OrientedBoundingBox.hpp"
#include "Vertex.hpp"
#include "engine/Device.hpp"
#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/buffers/vector/DeviceVector.hpp"
#include "engine/buffers/vector/HostVector.hpp"
#include "engine/texture/Texture.hpp"
#include "engine/utils.hpp"

namespace fgl::engine
{

	struct ModelMatrixInfo
	{
		glm::mat4 model_matrix;
		std::uint32_t texture_idx;
	};

	using VertexBufferSuballocation = DeviceVector< Vertex >;

	using IndexBufferSuballocation = DeviceVector< std::uint32_t >;

	using DrawParameterBufferSuballocation = HostVector< vk::DrawIndexedIndirectCommand >;

	using ModelMatrixInfoBufferSuballocation = HostVector< ModelMatrixInfo >;

	struct Primitive
	{
		VertexBufferSuballocation m_vertex_buffer;
		IndexBufferSuballocation m_index_buffer;
		OrientedBoundingBox< CoordinateSpace::Model > m_bounding_box;

		std::optional< Texture > m_texture { std::nullopt };

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			OrientedBoundingBox< CoordinateSpace::Model >& bounding_box ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box )
		{}

		Primitive(
			VertexBufferSuballocation&& vertex_buffer,
			IndexBufferSuballocation&& index_buffer,
			OrientedBoundingBox< CoordinateSpace::Model >& bounding_box,
			Texture&& texture ) :
		  m_vertex_buffer( std::move( vertex_buffer ) ),
		  m_index_buffer( std::move( index_buffer ) ),
		  m_bounding_box( bounding_box ),
		  m_texture( std::move( texture ) )
		{}

		Primitive() = delete;
		Primitive( const Primitive& other ) = delete;
		Primitive( Primitive&& other ) = default;
	};

	struct ModelBuilder
	{
		Buffer& m_vertex_buffer;
		Buffer& m_index_buffer;

		std::vector< ::fgl::engine::Primitive > m_primitives {};

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

		OrientedBoundingBox< CoordinateSpace::World > getBoundingBox( const Matrix< MatrixType::ModelToWorld > matrix )
			const
		{
			return matrix * m_bounding_box;
		}

		std::vector< ::fgl::engine::Primitive > m_primitives {};

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