//
// Created by kj16609 on 2/14/25.
//
#include "CommandBuffer.hpp"

#include "CommandBufferPool.hpp"

namespace fgl::engine
{
	CommandBufferHandle::CommandBufferHandle( vk::raii::CommandBuffer&& buffer, const CommandType type ) :
	  m_cmd_buffer( std::move( buffer ) ),
	  m_type( type )
	{}

	CommandBufferHandle::~CommandBufferHandle()
	{}

	vk::raii::CommandBuffer& CommandBufferHandle::cmd()
	{
		return m_cmd_buffer;
	}
} // namespace fgl::engine