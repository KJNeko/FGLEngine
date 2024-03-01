//
// Created by kj16609 on 2/27/24.
//

#pragma once

#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{

	class Vector;

	template < CoordinateSpace CType >
	class PointPlane
	{
		Coordinate< CType > coordinate;
		NormalVector vector;

	  public:

		constexpr static auto SpaceType { CType };

		PointPlane();

		PointPlane( const Coordinate< CType > pos, const Vector vec );

		PointPlane( const Coordinate< CType > pos, const NormalVector vec );

		NormalVector FGL_FORCE_INLINE getDirection() const { return vector; }

		float distance() const;

		Coordinate< CType > FGL_FORCE_INLINE getPosition() const { return coordinate; }

		float distanceFrom( const Coordinate< CType > coord ) const;

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

} // namespace fgl::engine
