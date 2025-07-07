//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "engine/FGL_DEFINES.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{
	class Vector;

	template < CoordinateSpace CType = CoordinateSpace::World >
	class OriginDistancePlane
	{
		float m_distance;
		NormalVector m_direction;

	  public:

		// ReSharper disable once CppInconsistentNaming
		constexpr static auto SpaceType { CType };

		OriginDistancePlane();

		explicit OriginDistancePlane( const NormalVector vector, const float distance );

		explicit OriginDistancePlane( const Vector& vector, const float distance );

		inline NormalVector getDirection() const { return m_direction; }

		//! Returns the closest point on the plane to the 0,0,0 origin
		Coordinate< CType > getPosition() const;

		float distanceFrom( const Coordinate< CType > coord ) const;

		FGL_FORCE_INLINE_FLATTEN bool isForward( const Coordinate< CType > coord ) const
		{
			return distanceFrom( coord ) > 0.0f;
		}

		FGL_FORCE_INLINE_FLATTEN bool isBehind( const Coordinate< CType > coord ) const { return !isForward( coord ); }

		//! Returns a normalized Vector
		NormalVector direction() const { return m_direction; }

		float distance() const { return m_distance; }

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	//	template < CoordinateSpace CType >
	//	using Plane = OriginDistancePlane< CType >;

} // namespace fgl::engine
