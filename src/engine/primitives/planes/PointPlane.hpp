//
// Created by kj16609 on 2/27/24.
//

#pragma once

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

		explicit PointPlane( const Coordinate< CType > pos, const Vector vec );

		explicit PointPlane( const Coordinate< CType > pos, const NormalVector vec ) : coordinate( pos ), vector( vec )
		{}

		explicit PointPlane( const glm::vec3 pos, const glm::vec3 vec ) : coordinate( pos ), vector( vec ) {}

		NormalVector getDirection() const { return vector; }

		float distance() const;

		Coordinate< CType > getPosition() const { return coordinate; }

		float distanceFrom( const Coordinate< CType > coord ) const
		{
			return static_cast< float >( glm::dot( ( coord - coordinate ).vec(), vector.vec() ) );
		}

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }
	};

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

} // namespace fgl::engine
