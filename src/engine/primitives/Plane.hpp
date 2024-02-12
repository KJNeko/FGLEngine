//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "Coordinate.hpp"
#include "Matrix.hpp"
#include "Vector.hpp"
#include "engine/constants.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType = CoordinateSpace::World >
	class Plane
	{
		float m_distance { constants::DEFAULT_FLOAT };
		Vector m_direction { constants::DEFAULT_VEC3 };

	  public:

		bool valid() const { return m_distance != constants::DEFAULT_FLOAT && m_direction != constants::DEFAULT_VEC3; }

		Plane( const glm::vec3 vector, const float distance ) : m_distance( distance ), m_direction( vector ) {}

		Plane( const glm::vec3 normal, const glm::vec3 point ) :
		  Plane( glm::normalize( normal ), glm::dot( glm::normalize( normal ), point ) )
		{}

		Plane( const Vector vector, const float distance ) : m_distance( distance ), m_direction( vector ) {}

		Plane() = default;

		/*
		Plane operator*( glm::mat4 matrix ) const
		{
			assert( valid() );

			Plane result = *this;
			const glm::vec3 new_direction { matrix * glm::vec4( m_direction, 1.0f ) };

			const float new_distance { glm::dot( new_direction, m_direction ) + m_distance };
			result.m_direction = glm::normalize( new_direction );
			result.m_distance = new_distance;

			return result;
		}*/

		//! Returns the closest point on the plane to the 0,0,0 origin
		Coordinate< CType > getPosition() const
		{
			assert( valid() );

			return Coordinate< CType >( 0.0f ) + ( m_direction * m_distance );
		}

		bool isForward( const WorldCoordinate coord ) const { return distanceFrom( coord ) > 0.0; }

		bool isBehind( const WorldCoordinate coord ) const { return !isForward( coord ); }

		//! Returns the distance from a point to the plane. Negative if behind, positive if in front
		double distanceFrom( const WorldCoordinate coord ) const
		{
			if constexpr ( CType == CoordinateSpace::World )
			{
				assert( valid() );
				return -( glm::dot( coord, m_direction ) - m_distance );
			}
			else
				throw std::runtime_error( "Plane must be in Plane<CoordinateType::World> to use distanceFrom" );
		}

		Vector direction() const { return m_direction; }

		float distance() const { return m_distance; }
	};

	template < CoordinateSpace CType, MatrixType MType >
	Plane< EvolvedType< MType >() > operator*( const Matrix< MType >& matrix, const Plane< CType >& plane )
	{
		constexpr auto NewCT { EvolvedType< MType >() };
		constexpr auto OldCT { CType };

		const Coordinate< OldCT > old_center { plane.getPosition() };

		//Translate old_center using matrix
		const Coordinate< NewCT > new_center { matrix * old_center };

		//Calculate distance between new_center and 0,0,0
		const float new_distance { glm::dot( plane.direction(), new_center ) };

		return { glm::normalize( new_center ), new_distance };
	}

} // namespace fgl::engine
