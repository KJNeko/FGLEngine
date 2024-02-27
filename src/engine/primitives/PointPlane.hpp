//
// Created by kj16609 on 2/27/24.
//

#pragma once

#include "Coordinate.hpp"
#include "CoordinateSpace.hpp"
#include "Line.hpp"
#include "Vector.hpp"

namespace fgl::engine
{

	template < CoordinateSpace CType >
	class PointPlane
	{
		Coordinate< CType > coordinate {};
		Vector vector {};

	  public:

		PointPlane() = default;

		PointPlane( const Coordinate< CType > pos, const Vector vec ) : coordinate( pos ), vector( vec ) {}

		explicit PointPlane( const glm::vec3 pos, const glm::vec3 vec ) : coordinate( pos ), vector( vec ) {}

		Vector direction() const { return vector; }

		float distance() const { return glm::dot( vector, coordinate ); }

		Coordinate< CType > getPosition() const { return coordinate; }

		float distanceFrom( const Coordinate< CType > coord ) const { return glm::dot( coord - coordinate, vector ); }

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }

		Coordinate< CType > intersection( const Line< CType > line ) const;
		Coordinate< CType > intersection( const Coordinate< CType > point, const Vector direction ) const;

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	PointPlane< EvolvedType< MType >() > operator*( const Matrix< MType >& mat, const PointPlane< CType >& plane )
	{
		PointPlane< EvolvedType< MType >() > new_plane { plane.getPosition() * mat, plane.direction() * mat };
		return new_plane;
	}

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

} // namespace fgl::engine
