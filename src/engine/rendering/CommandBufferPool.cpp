//
// Created by kj16609 on 2/14/25.
//
#include "CommandBufferPool.hpp"

#include "devices/Device.hpp"

namespace fgl::engine
{
	/*
	std::shared_ptr< CommandBufferHandle > CommandBufferPool::createBuffer()
	{
		CommandBufferHandle* handle_ptr { nullptr };

		vk::CommandBufferAllocateInfo allocate_info {};

		allocate_info.commandBufferCount = 1;

		return std::shared_ptr< CommandBufferHandle >( handle_ptr );
	}
	*/

	// No longer need eTransient as these will persist from start until destruction of the engine.
	// eResetCommandBuffer indicates that we'll be the ones resetting the command buffer.
	constexpr vk::CommandPoolCreateFlags CREATE_FLAGS { vk::CommandPoolCreateFlagBits::eResetCommandBuffer };

	CommandBuffer::CommandBuffer( std::shared_ptr< CommandBufferHandle >&& handle, CommandBufferPool* pool ) :
	  m_handle( std::move( handle ) ),
	  m_pool( pool )
	{}

	vk::raii::CommandBuffer* CommandBuffer::operator->() const
	{
		return &m_handle->cmd();
	}

	vk::raii::CommandBuffer& CommandBuffer::operator*() const
	{
		return m_handle->cmd();
	}

	void CommandBuffer::setName( const char* name )
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.setObjectType( vk::ObjectType::eCommandBuffer );
		info.setPObjectName( name );
		info.setObjectHandle(
			reinterpret_cast< std::uint64_t >( static_cast< VkCommandBuffer >( *( m_handle->cmd() ) ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

	CommandBuffer::CommandBuffer( CommandBuffer&& other ) noexcept :
	  m_handle( std::move( other.m_handle ) ),
	  m_pool( other.m_pool )
	{
		FGL_ASSERT( m_handle, "Invalid handle when moving" );
		other.m_handle = nullptr;
		other.m_pool = nullptr;
	}

	CommandBuffer::~CommandBuffer()
	{
		if ( m_handle ) m_pool->markInFlight( std::move( m_handle ) );
	}

	void CommandBufferPool::advanceInFlight()
	{
		std::lock_guard guard { m_queue_mtx };

		in_flight_idx += 1;
		if ( in_flight_idx >= constants::MAX_FRAMES_IN_FLIGHT ) in_flight_idx = 0;

		std::vector< std::shared_ptr< CommandBufferHandle > >& vec { m_in_flight[ in_flight_idx ] };

		// return all command buffers back to the pool to prepare for new insertions
		for ( std::shared_ptr< CommandBufferHandle >& buffer : vec )
		{
			// Reset command buffer before returning it back to the pool
			buffer->cmd().reset();

			FGL_ASSERT( buffer->m_type != CommandBufferHandle::Invalid, "Command buffer type invalid" );

			auto& target { buffer->m_type == CommandBufferHandle::Primary ? m_available_p_buffers :
				                                                            m_available_s_buffers };

			target.push( std::move( buffer ) );
		}

		vec.clear();
	}

	void CommandBufferPool::markInFlight( std::shared_ptr< CommandBufferHandle >&& buffer )
	{
		FGL_ASSERT( buffer, "Buffer was not valid!" );
		m_in_flight[ in_flight_idx ].emplace_back( std::move( buffer ) );
	}

	CommandBufferPool createGraphicsPool( vk::raii::Device& device, PhysicalDevice& physical_device )
	{
		const auto graphics_index { physical_device.queueInfo().getIndex( vk::QueueFlagBits::eGraphics ) };

		return { device, graphics_index };
	}

	CommandBufferPool createTransferPool( vk::raii::Device& device, PhysicalDevice& physical_device )
	{
		const auto transfer_index { physical_device.queueInfo().getIndex( vk::QueueFlagBits::eTransfer ) };

		return { device, transfer_index };
	}

	CommandBufferPool::CommandBufferPool( vk::raii::Device& device, const std::uint32_t queue_index ) :
	  m_pool_info( CREATE_FLAGS, queue_index ),
	  m_pool( device.createCommandPool( m_pool_info ) ),
	  in_flight_idx( 0 )
	{
		constexpr std::size_t command_buffer_target { 8 };
		constexpr std::size_t secondary_multip { 16 };

		vk::CommandBufferAllocateInfo info {};

		info.setCommandPool( m_pool );

		{
			info.setCommandBufferCount( command_buffer_target );
			info.setLevel( vk::CommandBufferLevel::ePrimary );

			std::vector< vk::raii::CommandBuffer > command_buffers { device.allocateCommandBuffers( info ) };

			std::lock_guard guard { m_queue_mtx };

			for ( auto& command_buffer : command_buffers )
			{
				auto* ptr { new CommandBufferHandle( std::move( command_buffer ), CommandBufferHandle::Primary ) };

				m_available_p_buffers.push( std::shared_ptr< CommandBufferHandle >( ptr ) );
			}
		}

		{
			info.setLevel( vk::CommandBufferLevel::eSecondary );
			info.setCommandBufferCount( command_buffer_target * secondary_multip );

			std::vector< vk::raii::CommandBuffer > command_buffers { device.allocateCommandBuffers( info ) };

			std::lock_guard guard { m_queue_mtx };

			for ( auto& command_buffer : command_buffers )
			{
				auto* ptr { new CommandBufferHandle( std::move( command_buffer ), CommandBufferHandle::Secondary ) };

				m_available_s_buffers.push( std::shared_ptr< CommandBufferHandle >( ptr ) );
			}
		}
	}

	CommandBufferPool::~CommandBufferPool()
	{}

	[[nodiscard]] CommandBuffer CommandBufferPool::getCommandBuffer( const CommandBufferHandle::CommandType type )
	{
		std::lock_guard guard { m_queue_mtx };

		auto& source { type == CommandBufferHandle::Primary ? m_available_p_buffers : m_available_s_buffers };

		FGL_ASSERT( source.size() > 0, "No available command buffers!" );

		auto buffer = source.front();
		source.pop();

		return { std::move( buffer ), this };
	}

	std::vector< CommandBuffer > CommandBufferPool::
		getCommandBuffers( const std::size_t count, const CommandBufferHandle::CommandType type )
	{
		std::lock_guard guard { m_queue_mtx };

		std::vector< CommandBuffer > command_buffers {};
		command_buffers.reserve( count );

		auto& source { type == CommandBufferHandle::Primary ? m_available_p_buffers : m_available_s_buffers };

		for ( std::size_t i = 0; i < count; ++i )
		{
			FGL_ASSERT( source.size() > 0, "No available command buffers!" );

			CommandBuffer buffer { std::move( source.front() ), this };

			command_buffers.emplace_back( std::move( buffer ) );

			source.pop();
		}

		return command_buffers;
	}

} // namespace fgl::engine
