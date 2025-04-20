//
// Created by kj16609 on 1/10/24.
//

#pragma once

#include "BufferHandle.hpp"
#include "FGL_DEFINES.hpp"
#include "engine/debug/Track.hpp"

namespace fgl::engine::descriptors
{
	class DescriptorSet;
}

namespace vk::raii
{
	class CommandBuffer;
}

namespace fgl::engine::memory
{
	class BufferHandle;

	struct BufferSuballocationHandle : public std::enable_shared_from_this< BufferSuballocationHandle >
	{
		Buffer m_parent_buffer;

		debug::Track< "GPU", "BufferSuballocationHandle" > m_track {};

		//! Size of the buffer this suballocation is a part of
		vk::DeviceSize m_size;

		//! Offset within buffer
		vk::DeviceSize m_offset;

		//! Alignment used when allocating
		vk::DeviceSize m_alignment;

		void* m_ptr { nullptr };

		bool m_staged { false };

		struct DescriptorInfo
		{
			std::weak_ptr< descriptors::DescriptorSet > m_descriptor {};

			enum class Type : std::uint8_t
			{
				Uniform,
				Storage,
				Array
			};

			Type m_type;

			union
			{
				struct
				{
					std::uint32_t m_binding_idx;
				} uniform_bind_info;

				struct
				{
					std::uint32_t m_binding_idx;
				} storage_bind_info;

				struct
				{
					std::size_t m_array_idx;
					std::size_t m_item_size;
					std::uint32_t m_binding_idx;
				} array_bind_info;
			};
		} m_descriptor_rebind_info;

		using enum DescriptorInfo::Type;

		BufferSuballocationHandle(
			const Buffer& p_buffer, vk::DeviceSize offset, vk::DeviceSize memory_size, vk::DeviceSize alignment );

		FGL_DELETE_DEFAULT_CTOR( BufferSuballocationHandle );
		FGL_DELETE_MOVE( BufferSuballocationHandle )
		FGL_DELETE_COPY_ASSIGN( BufferSuballocationHandle );

	  private:

		BufferSuballocationHandle( const BufferSuballocationHandle& other ) noexcept;

	  public:

		~BufferSuballocationHandle();

		[[nodiscard]] vk::Buffer getBuffer() const;
		[[nodiscard]] vk::Buffer getVkBuffer() const;
		void flush() const;

		void rebindDescriptor();

		/**
		 * @brief
		 * @param target
		 * @param target_offset Target offset into the suballocated range
		 * @param source_offset Source offset into the suballocated range
		 * @return
		 */
		[[nodiscard]] vk::BufferCopy copyRegion(
			const BufferSuballocationHandle& target, vk::DeviceSize target_offset, vk::DeviceSize source_offset ) const;

		[[nodiscard]] vk::DeviceSize getOffset() const { return m_offset; }

		void copyTo(
			const vk::raii::CommandBuffer& cmd_buffer,
			const BufferSuballocationHandle& target,
			std::size_t target_offset ) const;

		std::shared_ptr< BufferSuballocationHandle >
			reallocInTarget( const std::shared_ptr< BufferHandle >& new_buffer );

		bool ready() const { return m_staged; }

		void setReady( const bool value ) { m_staged = value; }
	};

} // namespace fgl::engine::memory
