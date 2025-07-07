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
		std::queue< T > m_unused_queue {};
		T m_current;

		T getNextID() { return m_current++; }

	  public:

		IDPool() = delete;

		IDPool( const T start_value ) : m_current( start_value ) {}

		void markUnused( const T value ) { m_unused_queue.push( value ); }

		constexpr static bool ALWAYS_NEW_ID { true };

		T getID()
		{
			if constexpr ( ALWAYS_NEW_ID )
			{
				return getNextID();
			}
			else
			{
				if ( m_unused_queue.empty() )
					return getNextID();
				else
				{
					const auto value { m_unused_queue.front() };
					m_unused_queue.pop();
					return value;
				}
			}
		}
	};
} // namespace fgl::engine