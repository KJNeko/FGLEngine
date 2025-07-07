//
// Created by kj16609 on 6/6/24.
//

#pragma once

#include <tracy/Tracy.hpp>

#include <memory>
#include <unordered_map>

namespace fgl::engine
{

	template < typename T >
	class AssetStore;

	/**
	 * @brief Interface class for Assets. Allows for usage in AssetStore<T>
	 * @tparam T
	 */
	template < typename T >
	struct AssetInterface
	{
		friend class AssetStore< T >;

		AssetInterface() = default;
		virtual ~AssetInterface() = default;
	};

	//! Concept for ensuring that the args given can extract a key for use in @ref AssetStore AssetStore<T>
	template < typename T, typename... TArgs >
	concept can_extract_key = requires( TArgs&&... args ) {
		{
			T::extractKey( std::declval< std::remove_reference_t< TArgs > >()... )
		} -> std::same_as< typename T::UIDKeyT >;
	};

	/**
	 * @brief Object store to keep track of assets.
	 * @tparam T Type to store T must define a `extractKey` method in T with the same parameters as the construction
	 * @copybrief
	 * T must:
	 * - Define UIDKeyT type inside of T.
	 * - Define and implement extractKey functions for each constructor used in load. (See can_extract_key)
	 */
	template < typename T >
	class AssetStore
	{
		static_assert(
			std::is_base_of_v< AssetInterface< T >, T >, "AssetStore<T>: T must inherit from AssetInterface" );

		//! Key type given by T
		using KeyT = typename T::UIDKeyT;

		std::unordered_map< KeyT, std::weak_ptr< T > > m_active_map {};
		std::mutex m_map_mtx {};

	  public:

		/**
		 * @brief Extracts the key supplied by the arguments (can_extract_key). Locates object in the map, If not in the map it is constructed with the arguments.
		 * @tparam T_Args Types to construct with
		 * @param args given args to construct T with
		 * @returns shared pointer to T
		 */
		template < typename... T_Args >
			requires can_extract_key< T, T_Args... >
		std::shared_ptr< T > load( T_Args&&... args )
		{
			ZoneScoped;
			const auto key { T::extractKey( std::forward< T_Args >( args )... ) };

			std::lock_guard guard { m_map_mtx };

			if ( auto itter = m_active_map.find( key ); itter != m_active_map.end() )
			{
				// We've found the item in the map. We can now check if it's still active

				if ( std::weak_ptr< T >& item = itter->second; !item.expired() )
				{
					return item.lock();
				}

				//Item was expired. Remove it from the map and continue
				m_active_map.erase( itter );
			}

			std::shared_ptr< T > s_ptr { new T( std::forward< T_Args >( args )... ) };

			// Add the weak pointer to the map so we can find it later.
			m_active_map.insert( std::make_pair( key, s_ptr ) );

			return s_ptr;
		}
	};

} // namespace fgl::engine
