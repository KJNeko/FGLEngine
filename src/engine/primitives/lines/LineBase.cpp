//
// Created by kj16609 on 2/29/24.
//

#include "LineBase.hpp"

#include <glm/geometric.hpp>

namespace fgl::engine
{

	glm::vec3 LineBase::planeIntersection( const glm::vec3 plane_vector, const float plane_distance ) const
	{
		const auto line_start { this->getVec3Position() };
		const auto line_vector { this->getVec3Direction() };

		const float line_dot { glm::dot( plane_vector, line_start ) };
		const float direction_dot { glm::dot( plane_vector, line_vector ) };

		// if the dot product of the direction of the plane and the direction of the line is zero, Then there will never be an intersection
		if ( direction_dot <= std::numeric_limits< float >::epsilon()
		     && direction_dot >= -std::numeric_limits< float >::epsilon() )
			return glm::vec3( std::numeric_limits< float >::quiet_NaN() );

		const float t { -( line_dot - plane_distance ) / direction_dot };

		const glm::vec3 intersection_point { line_start + ( t * line_vector ) };

		return intersection_point;
	}

} // namespace fgl::engine
