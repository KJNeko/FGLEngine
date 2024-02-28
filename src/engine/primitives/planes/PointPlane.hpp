//
// Created by kj16609 on 2/27/24.
//

#pragma once

#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class PointPlane
	{
		Coordinate< CType > coordinate;
		NormalVector vector;

	  public:

		explicit PointPlane( const Coordinate< CType > pos, const Vector vec ) : coordinate( pos ), vector( vec ) {}

		explicit PointPlane( const Coordinate< CType > pos, const NormalVector vec ) : coordinate( pos ), vector( vec )
		{}

		explicit PointPlane( const glm::vec3 pos, const glm::vec3 vec ) : coordinate( pos ), vector( vec ) {}

		NormalVector getDirection() const { return vector; }

		float distance() const { return glm::dot( vector, coordinate ); }

		Coordinate< CType > getPosition() const { return coordinate; }

		float distanceFrom( const Coordinate< CType > coord ) const
		{
			return static_cast< float >( glm::dot( coord - coordinate, vector ) );
		}

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }

		//TODO: Add
		bool intersects( const LineSegment< CType > line ) { return isForward( line.start ) != isForward( line.end ); }

		Coordinate< CType > intersection( const LineSegment< CType > line ) const;
		Coordinate< CType > intersection( const Coordinate< CType > point, const NormalVector direction ) const;

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	PointPlane< EvolvedType< MType >() > operator*( const Matrix< MType >& mat, const PointPlane< CType >& plane )
	{
		return PointPlane< EvolvedType< MType >() >( mat * plane.getPosition(), mat * plane.getDirection() );
	}

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

} // namespace fgl::engine
