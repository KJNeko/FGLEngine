//
// Created by kj16609 on 2/14/25.
//
#pragma once
#include <memory>
#include <queue>

#include "CommandBuffer.hpp"
#include "FGL_DEFINES.hpp"
#include "constants.hpp"
#include "devices/PhysicalDevice.hpp"
#include "types.hpp"

namespace fgl::engine
{
	class Device;

	class CommandBuffer
	{
		std::shared_ptr< CommandBufferHandle > m_handle;
		CommandBufferPool* m_pool;

		CommandBuffer( std::shared_ptr< CommandBufferHandle >&& handle, CommandBufferPool* pool );

		friend class CommandBufferPool;

	  public:

		vk::raii::CommandBuffer* operator->() const;

		vk::raii::CommandBuffer& operator*() const;

		void setName( const char* name );

		FGL_DELETE_COPY( CommandBuffer );
		FGL_DELETE_MOVE_ASSIGN( CommandBuffer );

		CommandBuffer( CommandBuffer&& other ) noexcept;

		~CommandBuffer();
	};

	class CommandBufferPool
	{
		vk::CommandPoolCreateInfo m_pool_info;
		vk::raii::CommandPool m_pool;

		std::mutex m_queue_mtx {};
		//! Available primary command buffers
		std::queue< std::shared_ptr< CommandBufferHandle > > m_available_s_buffers {};

		//! Available secondary command buffers
		std::queue< std::shared_ptr< CommandBufferHandle > > m_available_p_buffers {};

		// std::shared_ptr< CommandBufferHandle > createBuffer();

		//! Command pools that might still be in flight.
		FrameIndex in_flight_idx;
		std::array< std::vector< std::shared_ptr< CommandBufferHandle > >, constants::MAX_FRAMES_IN_FLIGHT >
			m_in_flight {};

		void markInFlight( std::shared_ptr< CommandBufferHandle >&& buffer );

		friend class CommandBuffer;

	  public:

		CommandBufferPool() = delete;
		CommandBufferPool( vk::raii::Device& device, std::uint32_t queue_index );
		// CommandBufferPool( PhysicalDevice& phy_device, vk::raii::Device& device );
		~CommandBufferPool();

		FGL_DELETE_COPY( CommandBufferPool );
		FGL_DELETE_MOVE( CommandBufferPool );

		//! Gets an available and empty command buffer
		//TODO: According to vulkan it's recomended to have a entire command pool per thread,
		// However because we are going to re-use as many of these buffers, I'm unsure if it's needed?
		CommandBuffer getCommandBuffer( CommandBufferHandle::CommandType type );
		std::vector< CommandBuffer > getCommandBuffers( std::size_t count, CommandBufferHandle::CommandType type );

		//! Moves the in-flight tracking number to the next index
		void advanceInFlight();
	};

	CommandBufferPool createGraphicsPool( vk::raii::Device& device, PhysicalDevice& physical_device );
	CommandBufferPool createTransferPool( vk::raii::Device& device, PhysicalDevice& physical_device );

} // namespace fgl::engine