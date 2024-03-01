//
// Created by kj16609 on 2/29/24.
//

#pragma once

namespace fgl::engine
{

	class LineBase
	{
		virtual glm::vec3 getVec3Position() const = 0;
		virtual glm::vec3 getVec3Direction() const = 0;

	  public:

		//This is pretty specific to each type of line. So i'll just leave this here for now.
		//bool intersectsPlane( const glm::vec3 plane_direction, const float plane_origin_distance ) const;

		glm::vec3 planeIntersection( const glm::vec3 plane_vector, const float plane_distance ) const;

		virtual ~LineBase() {}
	};

} // namespace fgl::engine
