//
// Created by kj16609 on 1/26/24.
//

#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

namespace fgl::engine
{
	constexpr static auto DEFAULT_COORDINATE_VEC3 { glm::vec3( std::numeric_limits< float >::max() ) };

	struct Coordinate
	{
		glm::vec3 position { DEFAULT_COORDINATE_VEC3 };

		Coordinate() = default;

		Coordinate( glm::vec3 position ) : position( position ) {}

		Coordinate( const Coordinate& other ) = default;

		Coordinate( Coordinate&& other ) : position( std::move( other.position ) )
		{
			other.position = DEFAULT_COORDINATE_VEC3;
		}
	};

	class Plane
	{
		float m_distance { std::numeric_limits< float >::max() };
		glm::vec3 m_direction { DEFAULT_COORDINATE_VEC3 };

	  public:

		bool valid() const
		{
			return m_distance != std::numeric_limits< float >::max() && m_direction != DEFAULT_COORDINATE_VEC3;
		}

		Plane( const glm::vec3 vector, const float distance ) : m_distance( distance ), m_direction( vector ) {}

		Plane( const glm::vec3 normal, const glm::vec3 point ) :
		  Plane( glm::normalize( normal ), glm::dot( glm::normalize( normal ), point ) )
		{}

		Plane() = default;

		Plane operator*( glm::mat4 matrix ) const;

		//! Returns the closest point on the plane to the 0,0,0 origin
		glm::vec3 getPosition() const
		{
			assert( m_distance != std::numeric_limits< float >::max() );
			assert( m_direction != DEFAULT_COORDINATE_VEC3 );
			return m_direction * m_distance;
		}

		bool isForward( const Coordinate& coord ) const;
		bool isBehind( const Coordinate& coord ) const;

		//! Returns the distance from a point to the plane. Negative if behind, positive if in front
		double distanceFrom( const Coordinate& coord ) const;

		glm::vec3 direction() const { return m_direction; }

		float distance() const { return m_distance; }
	};

	struct Frustum
	{
		Plane near;
		Plane far;

		Plane top;
		Plane bottom;

		Plane right;
		Plane left;

		friend class Camera;

	  public:

		Frustum() = default;

		Frustum(
			const Plane near_plane,
			const Plane far_plane,
			const Plane top_plane,
			const Plane bottom_plane,
			const Plane right_plane,
			const Plane left_plane ) :
		  near( near_plane ),
		  far( far_plane ),
		  top( top_plane ),
		  bottom( bottom_plane ),
		  right( right_plane ),
		  left( left_plane )
		{
			assert( near.valid() );
			assert( far.valid() );
			assert( top.valid() );
			assert( bottom.valid() );
			assert( right.valid() );
			assert( left.valid() );

			assert( right_plane.direction() != left_plane.direction() );
			assert( top_plane.direction() != bottom_plane.direction() );
			assert( near_plane.direction() != far_plane.direction() );
		}

		Frustum operator*( glm::mat4 matrix ) const
		{
			Frustum result = *this;
			result.near = near * matrix;
			result.far = far * matrix;
			result.top = top * matrix;
			result.bottom = bottom * matrix;
			result.right = right * matrix;
			result.left = left * matrix;

			return result;
		}

		bool pointInside( const Coordinate& coord ) const;
	};
} // namespace fgl::engine