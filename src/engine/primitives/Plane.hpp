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

#include "Coordinate.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "engine/constants.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType = CoordinateSpace::World >
	class OriginDistancePlane
	{
		float m_distance { constants::DEFAULT_FLOAT };
		Vector m_direction { constants::DEFAULT_VEC3 };

	  public:

		bool valid() const { return m_distance != constants::DEFAULT_FLOAT && m_direction != constants::DEFAULT_VEC3; }

		OriginDistancePlane( const glm::vec3 vector, const float distance ) :
		  m_distance( distance ),
		  m_direction( glm::normalize( vector ) )
		{}

		OriginDistancePlane( const Vector vector, const float distance ) :
		  m_distance( distance ),
		  m_direction( glm::normalize( vector ) )
		{}

		OriginDistancePlane() = default;

		//! Returns the closest point on the plane to the 0,0,0 origin
		Coordinate< CType > getPosition() const
		{
			assert( valid() );
			return Coordinate< CType >( 0.0f ) + ( m_direction * m_distance );
		}

		//! Returns the distance from a point to the plane. Negative if behind, positive if in front
		double distanceFrom( const WorldCoordinate coord ) const;

		bool isForward( const WorldCoordinate coord ) const { return distanceFrom( coord ) > 0.0; }

		bool isBehind( const WorldCoordinate coord ) const { return !isForward( coord ); }

		//! Returns a normalized Vector
		Vector direction() const
		{
			assert( m_direction != constants::DEFAULT_VEC3 );
			return m_direction;
		}

		float distance() const
		{
			assert( m_distance != constants::DEFAULT_FLOAT );
			return m_distance;
		}

		bool operator==( const OriginDistancePlane& other ) const
		{
			return m_distance == other.m_distance && m_direction == other.m_direction;
		}
	};

	template < CoordinateSpace CType >
	inline std::ostream& operator<<( std::ostream& os, const OriginDistancePlane< CType > plane )
	{
		os << "Plane: " << plane.direction() << " " << plane.distance();
		return os;
	}

	inline std::ostream& operator<<( std::ostream& os, const Vector vector )
	{
		os << "(" << vector.x << ", " << vector.y << ", " << vector.z << ")";
		return os;
	}

	template < CoordinateSpace CType, MatrixType MType >
	OriginDistancePlane< EvolvedType< MType >() >
		operator*( const Matrix< MType >& matrix, const OriginDistancePlane< CType >& plane )
	{
		assert( plane.valid() );
		constexpr auto NewCT { EvolvedType< MType >() };
		constexpr auto OldCT { CType };

		const Vector old_direction { plane.direction() };
		const Vector new_direction { glm::normalize( matrix * old_direction ) };

		const Coordinate< OldCT > old_center { plane.getPosition() };

		//Translate old_center using matrix
		const Coordinate< NewCT > new_center { matrix * old_center };

		//Project new_center onto inverse of new_direction
		const auto new_distance { static_cast< float >( glm::dot( new_direction, new_center ) ) };

		return { new_direction, new_distance };
	}

	template < CoordinateSpace CType >
	using Plane = OriginDistancePlane< CType >;

} // namespace fgl::engine
