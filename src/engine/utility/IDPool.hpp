//
// Created by kj16609 on 10/5/24.
//

#pragma once

#include <queue>

namespace fgl::engine
{

	template < typename T >
	class IDPool
	{
		std::queue< T > unused_queue {};
		T current;

		T getNextID() { return current++; }

	  public:

		IDPool() = delete;

		IDPool( const T start_value ) : current( start_value ) {}

		void markUnused( const T value ) { unused_queue.push( value ); }

		constexpr static bool ALWAYS_NEW_ID { true };

		T getID()
		{
			if constexpr ( ALWAYS_NEW_ID )
			{
				return getNextID();
			}
			else
			{
				if ( unused_queue.empty() )
					return getNextID();
				else
				{
					const auto value { unused_queue.front() };
					unused_queue.pop();
					return value;
				}
			}
		}
	};
} // namespace fgl::engine