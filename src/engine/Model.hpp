//
// Created by kj16609 on 11/28/23.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <algorithm>
#include <filesystem>
#include <memory>
#include <vector>

#include "Device.hpp"
#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/BufferSuballocation.hpp"
#include "engine/buffers/vector/DeviceVector.hpp"
#include "engine/buffers/vector/HostVector.hpp"
#include "utils.hpp"

namespace fgl::engine
{

	struct Vertex
	{
		glm::vec3 m_position { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_color { 1.0f, 1.0f, 1.0f };
		glm::vec3 m_normal { 0.0f, 0.0f, 0.0f };
		glm::vec2 m_uv { 0.0f, 0.0f };

		static std::vector< vk::VertexInputBindingDescription > getBindingDescriptions();
		static std::vector< vk::VertexInputAttributeDescription > getAttributeDescriptions();

		bool operator==( const Vertex& other ) const
		{
			return m_position == other.m_position && m_color == other.m_color && m_normal == other.m_normal
			    && m_uv == other.m_uv;
		}
	};

	struct ModelMatrixInfo
	{
		glm::mat4 model_matrix;
		glm::mat4 normal_matrix;
	};

	using VertexBufferSuballocation = DeviceVector< Vertex >;

	using IndexBufferSuballocation = DeviceVector< std::uint32_t >;

	using DrawParameterBufferSuballocation = HostVector< vk::DrawIndexedIndirectCommand >;

	using ModelMatrixInfoBufferSuballocation = HostVector< ModelMatrixInfo >;

	class Model
	{
		Device& m_device;
		VertexBufferSuballocation m_vertex_buffer;

		bool has_index_buffer { false };
		IndexBufferSuballocation m_index_buffer;

		vk::DrawIndexedIndirectCommand
			buildParameters( VertexBufferSuballocation& vertex_buffer, IndexBufferSuballocation& index_buffer );

		vk::DrawIndexedIndirectCommand m_draw_parameters;

	  public:

		struct Builder
		{
			std::vector< Vertex > verts {};
			std::vector< std::uint32_t > indicies {};
			Buffer& m_vertex_buffer;
			Buffer& m_index_buffer;

			Builder() = delete;

			Builder( Buffer& parent_vertex_buffer, Buffer& parent_index_buffer ) :
			  m_vertex_buffer( parent_vertex_buffer ),
			  m_index_buffer( parent_index_buffer )
			{}

			void loadModel( const std::filesystem::path& filepath );
		};

		vk::DrawIndexedIndirectCommand getDrawCommand() const { return m_draw_parameters; }

		static std::unique_ptr< Model > createModel(
			Device& device, const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer );

		void syncBuffers( vk::CommandBuffer& cmd_buffer );

		void bind( vk::CommandBuffer& cmd_buffer );
		void draw( vk::CommandBuffer& cmd_buffer );

		Model( Device& device, const Builder& builder );

		~Model() = default;

		Model( const Model& model ) = delete;
		Model& operator=( const Model& other ) = delete;
		Model( Model&& other ) = delete;
	};

} // namespace fgl::engine