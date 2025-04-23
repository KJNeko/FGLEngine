//
// Created by kj16609 on 4/20/25.
//
#pragma once
#include "descriptors/DescriptorSet.hpp"

namespace fgl::engine
{
	class Image;

	/**
	 * @brief Resource watcher is used to keep currently active resources (in use for the current frame, or previous frames, active, and does not allow them to be destroyed)
	 */
	struct ResourceWatcher
	{
		using ResourceVariant = std::variant<
			std::shared_ptr< descriptors::DescriptorSet >,
			std::shared_ptr< memory::Buffer >,
			std::shared_ptr< memory::BufferSuballocationHandle >,
			std::shared_ptr< Image > >;

		std::vector< ResourceVariant > m_resources;
		std::mutex m_mutex;
	};

	namespace watcher
	{
		ResourceWatcher* get();

		template < typename T >
			requires std::is_constructible_v< ResourceWatcher::ResourceVariant, T& >
		FGL_FORCE_INLINE void registerUse( T&& t )
		{
			auto* const ref { get() };
			std::lock_guard guard { ref->m_mutex };
			ref->m_resources.emplace_back( std::forward< T >( t ) );
		}

		void clearCurrent();

	} // namespace watcher

} // namespace fgl::engine