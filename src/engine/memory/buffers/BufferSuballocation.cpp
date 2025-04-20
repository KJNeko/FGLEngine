//
// Created by kj16609 on 1/10/24.
//

#include "BufferSuballocation.hpp"

#include "BufferSuballocationHandle.hpp"
#include "SuballocationView.hpp"
#include "align.hpp"
#include "engine/debug/logging/logging.hpp"

namespace fgl::engine::memory
{

	BufferSuballocation::BufferSuballocation( std::shared_ptr< BufferSuballocationHandle > handle ) :
	  m_handle( std::move( handle ) )
	{
		if ( handle.use_count() > 30 ) throw std::runtime_error( "AAAAAAAAA" );
	}

	BufferSuballocation& BufferSuballocation::operator=( BufferSuballocation&& other ) noexcept
	{
		m_handle = std::move( other.m_handle );

		other.m_handle = nullptr;

		return *this;
	}

	BufferSuballocation::BufferSuballocation( BufferSuballocation&& other ) noexcept :
	  m_handle( std::move( other.m_handle ) )
	{
		other.m_handle = nullptr;
	}

	void* BufferSuballocation::ptr() const
	{
		assert( m_handle != nullptr );
		FGL_ASSERT( m_handle->m_ptr, "Buffer must be mappable to use `ptr()`" );
		return m_handle->m_ptr;
	}

	void BufferSuballocation::flush() const
	{
		m_handle->flush();
	}

	Buffer& BufferSuballocation::getBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->m_parent_buffer;
	}

	vk::Buffer BufferSuballocation::getVkBuffer() const
	{
		assert( m_handle != nullptr );

		return m_handle->m_parent_buffer->getVkBuffer();
	}

	vk::DescriptorBufferInfo BufferSuballocation::descriptorInfo( const std::size_t byte_offset ) const
	{
		assert( !std::isnan( getOffset() ) );
		assert( !std::isnan( bytesize() ) );

		FGL_ASSERT( byte_offset < bytesize(), "Byte offset was greater then byte size!" );
		FGL_ASSERT(
			getOffset() + byte_offset < this->getBuffer()->size(),
			"Byte offset + buffer offset was greater then parent buffer size" );

		return { getVkBuffer(), getOffset() + byte_offset, bytesize() };
	}

	void BufferSuballocation::setRebindInfoStorage(
		const std::shared_ptr< descriptors::DescriptorSet >& descriptor, const std::uint32_t binding_idx ) const
	{
		m_handle->m_descriptor_rebind_info.m_descriptor = descriptor;
		m_handle->m_descriptor_rebind_info.m_type = BufferSuballocationHandle::Storage;
		m_handle->m_descriptor_rebind_info.storage_bind_info.m_binding_idx = binding_idx;
	}

	void BufferSuballocation::setRebindInfoUniform(
		const std::shared_ptr< descriptors::DescriptorSet >& descriptor, const std::uint32_t binding_idx ) const
	{
		m_handle->m_descriptor_rebind_info.m_descriptor = descriptor;
		m_handle->m_descriptor_rebind_info.m_type = BufferSuballocationHandle::Uniform;
		m_handle->m_descriptor_rebind_info.uniform_bind_info.m_binding_idx = binding_idx;
	}

	void BufferSuballocation::setRebindInfoArray(
		const std::uint32_t binding_idx,
		const std::shared_ptr< descriptors::DescriptorSet >& descriptor,
		const std::size_t array_idx,
		const std::size_t item_size ) const
	{
		m_handle->m_descriptor_rebind_info.m_descriptor = descriptor;
		m_handle->m_descriptor_rebind_info.m_type = BufferSuballocationHandle::Array;
		m_handle->m_descriptor_rebind_info.array_bind_info.m_array_idx = array_idx;
		m_handle->m_descriptor_rebind_info.array_bind_info.m_item_size = item_size;
		m_handle->m_descriptor_rebind_info.array_bind_info.m_binding_idx = binding_idx;
	}

	BufferSuballocation::~BufferSuballocation() = default;

	SuballocationView BufferSuballocation::view( const vk::DeviceSize offset, const vk::DeviceSize size ) const
	{
		assert( m_handle != nullptr );
		assert( offset + size <= bytesize() && "BufferSuballocation::view() called with offset + size > m_size" );

		return { m_handle, offset, size };
	}

	BufferSuballocation::BufferSuballocation( const Buffer& buffer, const vk::DeviceSize size ) :
	  BufferSuballocation( buffer->allocate( size ) )
	{}

	BufferSuballocation::
		BufferSuballocation( const Buffer& buffer, const std::size_t t_size, const std::uint32_t t_align ) :
	  BufferSuballocation( buffer->allocate( t_size, t_align ) )
	{}

} // namespace fgl::engine::memory
