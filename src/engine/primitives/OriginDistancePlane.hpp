//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <stdexcept>
#include <utility>

#include "Vector.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/point/Coordinate.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType = CoordinateSpace::World >
	class OriginDistancePlane
	{
		float m_distance { constants::DEFAULT_FLOAT };
		Vector m_direction { constants::WORLD_FORWARD };

	  public:

		OriginDistancePlane() = default;

		OriginDistancePlane( const glm::vec3 vector, const float distance ) :
		  m_distance( distance ),
		  m_direction( glm::normalize( vector ) )
		{}

		OriginDistancePlane( const Vector vector, const float distance ) :
		  m_distance( distance ),
		  m_direction( glm::normalize( vector ) )
		{}

		//! Returns the closest point on the plane to the 0,0,0 origin
		Coordinate< CType > getPosition() const { return Coordinate< CType >( 0.0f ) + ( m_direction * m_distance ); }

		//! Returns the distance from a point to the plane. Negative if behind, positive if in front
		float distanceFrom( const WorldCoordinate coord ) const;

		bool isForward( const WorldCoordinate coord ) const { return distanceFrom( coord ) > 0.0f; }

		bool isBehind( const WorldCoordinate coord ) const { return !isForward( coord ); }

		//! Returns a normalized Vector
		Vector direction() const { return m_direction; }

		float distance() const { return m_distance; }

		bool operator==( const OriginDistancePlane& other ) const
		{
			return m_distance == other.m_distance && m_direction == other.m_direction;
		}

		bool intersects( const LineSegment< CType > line ) const;

		Coordinate< CType > intersection( const LineSegment< CType > line ) const;
		Coordinate< CType > intersection( const Coordinate< CType > point, const Vector direction ) const;

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	OriginDistancePlane< EvolvedType< MType >() >
		operator*( const Matrix< MType >& matrix, const OriginDistancePlane< CType >& plane )
	{
		const Vector new_direction { glm::normalize( matrix * plane.direction() ) };
		const glm::vec3 new_center { matrix * plane.getPosition() };
		const float new_distance { glm::dot( new_center, static_cast< glm::vec3 >( new_direction ) ) };

		return { new_direction, new_distance };
	}

	//	template < CoordinateSpace CType >
	//	using Plane = OriginDistancePlane< CType >;

} // namespace fgl::engine
