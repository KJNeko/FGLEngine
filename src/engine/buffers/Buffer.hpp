//
// Created by kj16609 on 11/30/23.
//

#pragma once

#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan.hpp>

#include <cassert>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <unordered_map>

#include "engine/align.hpp"
#include "engine/literals/size.hpp"
#include "vma/vma_impl.hpp"

namespace fgl::engine
{

	class BufferHandle;

	struct BufferSuballocationInfo
	{
		std::shared_ptr< BufferHandle > buffer;
		vk::DeviceSize offset;
		vk::DeviceSize size;
	};

	//TODO: Dynamic/onDemand resizing of Buffer for suballocations
	//TODO: Defragmentation

	class Device;

	class BufferHandle
	{
		vk::Buffer m_buffer;
		VmaAllocation m_allocation {};
		VmaAllocationInfo m_alloc_info;

		vk::DeviceSize m_memory_size;

		vk::BufferUsageFlags m_usage;
		vk::MemoryPropertyFlags m_memory_properties;

		void alloc( vk::DeviceSize memory_size );
		void dealloc();

	  public:

		BufferHandle() = delete;
		BufferHandle( const BufferHandle& other ) = delete;
		BufferHandle& operator=( const BufferHandle& other ) = delete;
		BufferHandle( BufferHandle&& other ) = delete;
		BufferHandle& operator=( BufferHandle&& other ) = delete;

		friend class Buffer;

		BufferHandle(
			vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		~BufferHandle();
	};

	class Buffer
	{
		std::shared_ptr< BufferHandle > m_handle;

	  public:

		vk::Buffer& getVkBuffer() { return m_handle->m_buffer; }

		vk::BufferUsageFlags m_usage;

		vk::MemoryPropertyFlags m_memory_properties;

		//! Returns the required alignment for this buffer.
		vk::DeviceSize alignment();

		~Buffer();
		Buffer() = delete;
		Buffer( vk::DeviceSize memory_size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags memory_properties );

		Buffer( const Buffer& other ) = delete;
		Buffer( Buffer&& other ) = delete;
		Buffer& operator=( const Buffer& other ) = delete;

	  private:

		//! @brief List of all active suballocations
		//! <offset, size>
		std::map< vk::DeviceSize, vk::DeviceSize > m_suballocations {};

		//! @brief list of any free blocks
		//! @note All blocks are amalgamated to the largest they can expand to.
		//! <offset, size>
		std::vector< std::pair< vk::DeviceSize, vk::DeviceSize > > m_free_blocks {};

	  public:

		//! Returns the vulkan buffer handle for this buffer
		vk::Buffer getBuffer() const { return m_handle->m_buffer; }

		//! Returns the vulkan memory handle for this buffer
		vk::DeviceMemory getMemory() const { return m_handle->m_alloc_info.deviceMemory; }

		//! Total memory size of the buffer
		vk::DeviceSize size() const { return m_handle->m_memory_size; }

		void* map( BufferSuballocationInfo& info )
		{
			assert(
				info.offset + info.size <= m_handle->m_memory_size
				&& "BufferSuballocationT::map() called with invalid size" );
			assert(
				m_handle->m_alloc_info.pMappedData
				&& "BufferSuballocationT::map() called on buffer with no mapped data" );

			return static_cast< std::byte* >( m_handle->m_alloc_info.pMappedData ) + info.offset;
		}

		bool isMappable() const { return m_handle->m_alloc_info.pMappedData != nullptr; }

		//! Returns a allocation block from this buffer. Block will be aligned with nonUniformBufferOffsetAlignment
		//! and nonCoherentAtomSize if required (is_uniform_buffer and is_host_visible respectively)
		/**
		 * @param memory_size Size of each N
		 * @param number of blocks to allocate. (Returns as 1 block with each N being an aligned offset)
		 * @param alignment The alignment to use.
		 * @return
		 *
		 * @note Alignment is forced to be at least the size of the largest alignment required by the device.
		 * (`alignment` vs `nonCoherentAtomSize` vs `minUniformBufferOffsetAlignment`)
		 * @par
		 * @note Alignment for ubo is 0 if the buffer is not a uniform buffer
		 * @par
		 * @note Alignment for atom_size is 0 if buffer is not host visible
		 */
		BufferSuballocationInfo suballocate( vk::DeviceSize memory_size, std::uint32_t allignment )
		{
			ZoneScoped;
			//Calculate alignment from alignment, ubo_alignment, and atom_size_alignment
			memory_size = align( memory_size, alignment() );

			//Find a free space.
			auto itter = std::find_if(
				m_free_blocks.begin(),
				m_free_blocks.end(),
				[ this, memory_size, allignment ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
				{
					auto [ offset, size ] = pair;

					const auto new_offset = align( offset, alignment(), allignment );
					size -= new_offset - offset;

					return size >= memory_size;
				} );

			if ( itter == m_free_blocks.end() )
			{
				std::cout << "======    !!! OOM !!!   ======\n"
							 "====== Allocated Blocks ======\n";

				for ( auto [ offset, size ] : m_free_blocks )
				{
					std::cout << "Offset: " << std::hex << offset << " Size: " << std::dec << size << "\n";

					std::cout << "Aligned offset: " << std::hex << align( offset, alignment(), allignment )
							  << " Size: " << std::dec << size << "\n"
							  << std::endl;
				}

				std::cout << "=============================\n"
						  << "Attempted to allocate block of size: "
						  << fgl::literals::size_literals::to_string( memory_size ) << std::endl;

				throw std::runtime_error( "Failed to find free space" );
			}

			//Allocate
			auto [ offset, size ] = *itter;
			m_free_blocks.erase( itter );

			const auto aligned_offset { align( offset, alignment(), allignment ) };

			if ( aligned_offset != offset )
			{
				m_free_blocks.emplace_back( std::make_pair( offset, aligned_offset - offset ) );
				offset = aligned_offset;
				size -= aligned_offset - offset;
			}

			m_suballocations.insert_or_assign( offset, memory_size );

			if ( size - memory_size > 0 )
				m_free_blocks.emplace_back( std::make_pair( offset + memory_size, size - memory_size ) );

			return { m_handle, offset, memory_size };
		}

		void free( BufferSuballocationInfo& info )
		{
			ZoneScoped;

			{
				//Find the suballocation
				auto itter = m_suballocations.find( info.offset );

				if ( itter == m_suballocations.end() ) throw std::runtime_error( "Failed to find suballocation" );

				//Remove the suballocation
				m_suballocations.erase( itter );
			}

			// Forward check
			{
				ZoneScopedN( "Forward check" );
				auto itter = std::find_if(
					m_free_blocks.begin(),
					m_free_blocks.end(),
					[ &info ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
					{
						auto& [ offset, size ] = pair;
						return offset > info.offset && offset == info.offset + info.size;
					} );

				//If itter is not end then we have found a block where itter->offset > offset

				if ( itter != m_free_blocks.end() )
				{
					auto& [ free_offset, free_size ] = *itter;
					info.size += free_size; // Add their size to ours

					//Nuke block
					m_free_blocks.erase( itter );
				}
			}

			// Backwards check
			{
				ZoneScopedN( "Backwards check" );
				auto prev_block = std::find_if(
					m_free_blocks.begin(),
					m_free_blocks.end(),
					[ &info ]( const std::pair< vk::DeviceSize, vk::DeviceSize >& pair )
					{
						auto& [ offset, size ] = pair;
						return offset + size + 1 == info.offset;
					} );

				if ( prev_block != m_free_blocks.end() )
				{
					auto& [ offset, size ] = *prev_block;
					size += info.size;
				}
				else
				{
					//No block before us. We are the free block
					m_free_blocks.push_back( { info.offset, info.size } );
				}
			}
		}
	};

	void initGlobalStagingBuffer( std::uint64_t size );
	Buffer& getGlobalStagingBuffer();
} // namespace fgl::engine
