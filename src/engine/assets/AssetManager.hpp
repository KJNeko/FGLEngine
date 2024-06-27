//
// Created by kj16609 on 6/6/24.
//

#pragma once

#include <tracy/Tracy.hpp>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <filesystem>
#include <memory>
#include <queue>

namespace fgl::engine
{

	template < typename T >
	class AssetStore;

	template < typename T >
	struct AssetInterface
	{
		friend class AssetStore< T >;

		AssetInterface() = default;
		virtual ~AssetInterface() = default;
	};

	template < typename T >
	class AssetStore
	{
		static_assert(
			std::is_base_of_v< AssetInterface< T >, T >, "AssetStore<T, TKey>: T must inherit from AssetInterface" );

		//! Items that are actively in use.
		//std::unordered_map< TKey, std::weak_ptr< T > > active_map {};

		//! Assets needing to be staged
		//TODO: ASYNC Ring buffer queue
		std::queue< std::shared_ptr< T > > to_stage {};
		//TODO: Add tracy monitor to mutex
		std::mutex queue_mtx {};

		//! Assets currently being staged.
		std::vector< std::shared_ptr< T > > processing {};

	  public:

		//TODO: Come up with a better design the the loading function.
		/// We should have a way to prevent a asset from being loaded multiple times.
		template < typename... T_Args >
		//TODO: This genuinely seems like a GCC Bug. Perhaps try to find a workaround later or report as such.
		//requires std::constructible_from< T, T_Args... >
		std::shared_ptr< T > load( T_Args&&... args )
		{
			ZoneScoped;
			std::lock_guard guard { queue_mtx };

			T* ptr { new T( std::forward< T_Args >( args )... ) };
			std::shared_ptr< T > s_ptr { ptr };

			to_stage.push( s_ptr );

			return s_ptr;
		}

		//! Returns true if all items to be staged were submitted to the queue
		//! Returns false if more items remain
		bool stage( vk::raii::CommandBuffer& buffer )
		{
			ZoneScoped;
			std::lock_guard guard { queue_mtx };
			//! Number of items to process during a stage step
			constexpr std::size_t max_count { 16 };

			for ( std::size_t i = 0; i < max_count; ++i )
			{
				if ( to_stage.empty() ) break;

				processing.emplace_back( to_stage.front() );
				to_stage.pop();
			}

			if ( processing.size() == 0 ) return true;

			for ( const auto& ptr : processing )
			{
				ptr->stage( buffer );
			}

			return to_stage.empty();
		}

		void confirmStaged()
		{
			for ( const auto& ptr : processing )
			{
				ptr->dropStaging();
				ptr->setReady();
			}

			//TODO: Map this into a weak ptr in order to prevent duplication.
			processing.clear();
		}
	};

} // namespace fgl::engine
