//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include "LineBase.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/planes/concepts.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class InfiniteLine : public LineBase
	{
		Coordinate< CType > m_start;
		NormalVector m_direction;

		glm::vec3 getVec3Position() const override { return getPosition().vec(); }

		glm::vec3 getVec3Direction() const override { return getDirection().vec(); }

	  public:

		InfiniteLine() = delete;

		InfiniteLine( const Coordinate< CType > point, const NormalVector vector ) :
		  m_start( point ),
		  m_direction( vector )
		{}

		NormalVector getDirection() const { return m_direction; }

		Coordinate< CType > getPosition() const { return m_start; }

		InfiniteLine operator-() const { return InfiniteLine( m_start, -m_direction ); }

		inline InfiniteLine flip() const { return -( *this ); }

		template < typename T >
			requires is_plane< T >
		FGL_FLATTEN bool intersects( const T plane ) const
		{
			return !std::isnan( glm::dot( plane.getDirection(), getDirection() ) );
		}

		template < typename T >
			requires is_plane< T >
		FGL_FLATTEN Coordinate< CType > intersection( const T plane ) const
		{
			return Coordinate< CType >( planeIntersection( plane.getDirection().vec(), plane.distance() ) );
		}
	};

} // namespace fgl::engine
