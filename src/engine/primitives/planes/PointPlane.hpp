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
		Coordinate< CType > m_coordinate;
		NormalVector m_vector;

	  public:

		// ReSharper disable once CppInconsistentNaming
		constexpr static auto SpaceType { CType };

		PointPlane();

		PointPlane( Coordinate< CType > pos, const Vector& vec );

		PointPlane( Coordinate< CType > pos, NormalVector vec );

		FGL_FORCE_INLINE NormalVector getDirection() const { return m_vector; }

		float distance() const;

		FGL_FORCE_INLINE Coordinate< CType > getPosition() const { return m_coordinate; }

		float distanceFrom( const Coordinate< CType > coord ) const;

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }

		Coordinate< CType > mapToPlane( const Coordinate< CType > point ) const;
	};

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

	using ModelPlane = Plane< CoordinateSpace::Model >;
	using WorldPlane = Plane< CoordinateSpace::World >;

} // namespace fgl::engine
