//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <concepts>

namespace glm
{
	template < typename T1, typename T2 >
		requires( std::is_base_of_v< glm::vec3, T1 > && std::is_base_of_v< glm::vec3, T2 > )
	float dot( const T1 t1, const T2 t2 )
	{
		return dot( static_cast< glm::vec3 >( t1 ), static_cast< glm::vec3 >( t2 ) );
	}

	template < typename T1 >
		requires( std::is_base_of_v< glm::vec3, T1 > )
	float dot( const T1 t1, const T1 t2 )
	{
		return dot( static_cast< glm::vec3 >( t1 ), static_cast< glm::vec3 >( t2 ) );
	}
} // namespace glm
