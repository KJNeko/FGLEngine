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

	/**
	 * @brief
	 * @tparam CType
	 */
	template < CoordinateSpace CType >
	class SimplePlane
	{
		NormalVector m_vector;
		float m_distance;

	  public:

		/**
		 * @param vector
		 * @param distance
		 * @return A new instance of a simple plane with the given dimensions.
		 */
		[[nodiscard]] SimplePlane( NormalVector vector, float distance ) : m_vector( vector ), m_distance( distance ) {}

		[[nodiscard]] NormalVector getDirection() const { return m_vector; }

		[[nodiscard]] float distance() const { return m_distance; }

		[[nodiscard]] float distanceFrom( const Coordinate< CType >& coord ) const
		{
			return glm::dot( m_vector.vec(), coord.vec() ) + m_distance;
		}

		[[nodiscard]] bool isForward( const Coordinate< CType >& coord ) const { return distanceFrom( coord ) > 0.0f; }

		[[nodiscard]] Coordinate< CType > mapToPlane( const Coordinate< CType >& point ) const
		{
			const float dist = distanceFrom( point );
			return Coordinate< CType >( point.vec() - dist * m_vector.vec() );
		}
	};

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

		FGL_FORCE_INLINE [[nodiscard]] NormalVector getDirection() const { return m_vector; }

		[[nodiscard]] float distance() const;

		FGL_FORCE_INLINE Coordinate< CType > getPosition() const { return m_coordinate; }

		float distanceFrom( Coordinate< CType > coord ) const;

		bool isForward( const Coordinate< CType > coord ) const { return distanceFrom( coord ) > 0.0f; }

		[[nodiscard]] Coordinate< CType > mapToPlane( Coordinate< CType > point ) const;

		[[nodiscard]] SimplePlane< CType > toSimple() const;
	};

	static_assert(
		sizeof( glm::vec4 ) == sizeof( SimplePlane< CoordinateSpace::World > ), "SimplePlane was not 4 floats" );

	template < CoordinateSpace CType >
	using Plane = PointPlane< CType >;

	using ModelPlane = Plane< CoordinateSpace::Model >;
	using WorldPlane = Plane< CoordinateSpace::World >;

} // namespace fgl::engine
