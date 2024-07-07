//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include "LineBase.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/planes/concepts.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	class LineSegment final : public LineBase
	{
		Coordinate< CType > start { -constants::DEFAULT_VEC3 };
		Coordinate< CType > end { constants::DEFAULT_VEC3 };

		glm::vec3 getVec3Position() const override { return getPosition().vec(); }

		glm::vec3 getVec3Direction() const override { return getDirection().vec(); }

	  public:

		LineSegment() = default;

		explicit LineSegment( const Coordinate< CType > i_start, const Coordinate< CType > i_end ) noexcept :
		  start( i_start ),
		  end( i_end )
		{}

		explicit LineSegment( const glm::vec3 i_start, glm::vec3 i_end ) : start( i_start ), end( i_end ) {}

		NormalVector getDirection() const { return start.normalTo( end ); }

		Coordinate< CType > getPosition() const { return start; }

		Coordinate< CType > getEnd() const { return end; }

		LineSegment flip() const { return LineSegment( end, start ); }

		template < typename T >
			requires is_plane< T >
		FGL_FLATTEN bool intersects( const T plane ) const
		{
			return plane.isForward( start ) != plane.isForward( end );
		}

		template < typename T >
		bool intersets( const T t ) const;

		template < typename T >
			requires is_plane< T >
		FGL_FLATTEN Coordinate< CType > intersection( const T plane ) const
		{
			return Coordinate< CType >( planeIntersection( plane.getDirection().vec(), plane.distance() ) );
		}
	};

	template < CoordinateSpace CType >
	using Line = LineSegment< CType >;

} // namespace fgl::engine
