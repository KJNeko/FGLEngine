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

#include "engine/constants.hpp"
#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType = CoordinateSpace::World >
	class OriginDistancePlane
	{
		float m_distance { constants::DEFAULT_FLOAT };
		NormalVector m_direction { constants::WORLD_FORWARD };

	  public:

		OriginDistancePlane() = default;

		explicit OriginDistancePlane( const NormalVector vector, const float distance ) :
		  m_distance( distance ),
		  m_direction( vector )
		{}

		//! Returns the closest point on the plane to the 0,0,0 origin
		Coordinate< CType > getPosition() const { return Coordinate< CType >( m_direction * m_distance ); }

		//! Returns the distance from a point to the plane. Negative if behind, positive if in front
		float distanceFrom( const WorldCoordinate coord ) const;

		bool isForward( const WorldCoordinate coord ) const { return distanceFrom( coord ) > 0.0f; }

		bool isBehind( const WorldCoordinate coord ) const { return !isForward( coord ); }

		//! Returns a normalized Vector
		NormalVector direction() const { return m_direction; }

		float distance() const { return m_distance; }

		bool operator==( const OriginDistancePlane& other ) const
		{
			return m_distance == other.m_distance && m_direction == other.m_direction;
		}

		bool intersects( const LineSegment< CType > line ) const;
		bool intersects( const InfiniteLine< CType > line ) const;

		Coordinate< CType > intersection( const LineSegment< CType > line ) const;
		Coordinate< CType > intersection( const InfiniteLine< CType > line ) const;

		Coordinate< CType > intersection( const Coordinate< CType > point, const NormalVector direction ) const;

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	//	template < CoordinateSpace CType >
	//	using Plane = OriginDistancePlane< CType >;

} // namespace fgl::engine
