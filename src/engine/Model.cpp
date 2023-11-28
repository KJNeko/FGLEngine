//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>
#include <cstring>

namespace fgl::engine
{

	Model::Model( Device& device, const std::vector< Vertex >& verts ) : m_device( device )
	{
		createVertexBuffers( verts );
	}

	Model::~Model()
	{
		vkDestroyBuffer( m_device.device(), m_vertex_buffer, nullptr );
		vkFreeMemory( m_device.device(), m_buffer_memory, nullptr );
	}

	void Model::createVertexBuffers( const std::vector< Vertex >& verts )
	{
		m_vertex_count = static_cast< std::uint32_t >( verts.size() );
		assert( verts.size() >= 3 && "Vertex count must be at least 3" );

		VkDeviceSize buffer_size { sizeof( verts[ 0 ] ) * m_vertex_count };

		m_device.createBuffer(
			buffer_size,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertex_buffer,
			m_buffer_memory );

		void* data { nullptr };
		vkMapMemory( m_device.device(), m_buffer_memory, 0, buffer_size, 0, &data );

		std::memcpy( data, verts.data(), static_cast< std::size_t >( buffer_size ) );
		vkUnmapMemory( m_device.device(), m_buffer_memory );
	}

	void Model::bind( VkCommandBuffer buffer )
	{
		VkBuffer buffers[] { m_vertex_buffer };
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers( buffer, 0, 1, buffers, offsets );
	}

	void Model::draw( VkCommandBuffer buffer )
	{
		vkCmdDraw( buffer, m_vertex_count, 1, 0, 0 );
	}

	std::vector< VkVertexInputBindingDescription > Model::Vertex::getBindingDescriptions()
	{
		std::vector< VkVertexInputBindingDescription > binding_descriptions {
			{ .binding = 0, .stride = sizeof( Vertex ), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX }
		};

		return binding_descriptions;
	}

	std::vector< VkVertexInputAttributeDescription > Model::Vertex::getAttributeDescriptions()
	{
		std::vector< VkVertexInputAttributeDescription > attribute_descriptions {
			{ .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof( Vertex, m_pos ) },
			{ .location = 1,
			  .binding = 0,
			  .format = VK_FORMAT_R32G32B32_SFLOAT,
			  .offset = offsetof( Vertex, m_color ) },
		};

		return attribute_descriptions;
	}
} // namespace fgl::engine