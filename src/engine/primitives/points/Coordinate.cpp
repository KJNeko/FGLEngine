//
// Created by kj16609 on 2/28/24.
//

#include "Coordinate.hpp"

#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{

	template <>
	Coordinate< CoordinateSpace::Model >
		operator+( const Coordinate< CoordinateSpace::Model > coord, const Vector vector )
	{
		return Coordinate<
			CoordinateSpace::Model >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( vector ) );
	}

	template <>
	Coordinate< CoordinateSpace::Model >
		operator+( const Coordinate< CoordinateSpace::Model > coord, const NormalVector vector )
	{
		return Coordinate<
			CoordinateSpace::Model >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( vector ) );
	}

	template <>
	Coordinate< CoordinateSpace::Model >
		operator+( const Coordinate< CoordinateSpace::Model > coord, const Coordinate< CoordinateSpace::Model > other )
	{
		return Coordinate<
			CoordinateSpace::Model >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( other ) );
	}

	template <>
	Coordinate< CoordinateSpace::Model >
		operator-( const Coordinate< CoordinateSpace::Model > coord, const Coordinate< CoordinateSpace::Model > other )
	{
		return Coordinate<
			CoordinateSpace::Model >( static_cast< glm::vec3 >( coord ) - static_cast< glm::vec3 >( other ) );
	}

	template <>
	Coordinate< CoordinateSpace::World >
		operator+( const Coordinate< CoordinateSpace::World > coord, const Vector vector )
	{
		return Coordinate<
			CoordinateSpace::World >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( vector ) );
	}

	template <>
	Coordinate< CoordinateSpace::World >
		operator+( const Coordinate< CoordinateSpace::World > coord, const NormalVector vector )
	{
		return Coordinate<
			CoordinateSpace::World >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( vector ) );
	}

	template <>
	Coordinate< CoordinateSpace::World >
		operator+( const Coordinate< CoordinateSpace::World > coord, const Coordinate< CoordinateSpace::World > other )
	{
		return Coordinate<
			CoordinateSpace::World >( static_cast< glm::vec3 >( coord ) + static_cast< glm::vec3 >( other ) );
	}

	template <>
	Coordinate< CoordinateSpace::World >
		operator-( const Coordinate< CoordinateSpace::World > coord, const Coordinate< CoordinateSpace::World > other )
	{
		return Coordinate<
			CoordinateSpace::World >( static_cast< glm::vec3 >( coord ) - static_cast< glm::vec3 >( other ) );
	}

} // namespace fgl::engine
