//
// Created by kj16609 on 2/15/24.
//

#pragma once

namespace glm
{
	inline void PrintTo( const glm::vec3& vec, ::std::ostream* os )
	{
		*os << "(" << vec.x << "," << vec.y << "," << vec.z << ")";
	}

} // namespace glm

namespace fgl::engine
{
	inline void PrintTo( const Vector& vec, ::std::ostream* os )
	{
		glm::PrintTo( static_cast< glm::vec3 >( vec ), os );
	}
} // namespace fgl::engine
