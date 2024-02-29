//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include <ostream>

#include "engine/constants.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/Scale.hpp"
#include "engine/primitives/glmOperators.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/matricies/MatrixEvolvedTypes.hpp"

namespace fgl::engine
{

	class Vector;
	class NormalVector;

	template < CoordinateSpace type >
	class Coordinate : public glm::vec3
	{
		using ValueT = float;

	  public:

		float& up() { return z; }

		float up() const { return z; }

		float& right() { return x; }

		float right() const { return x; }

		float& forward() { return y; }

		float forward() const { return y; }

		Coordinate() noexcept : glm::vec3( constants::DEFAULT_VEC3 ) {}

		explicit Coordinate( const glm::vec3 position ) noexcept : glm::vec3( position ) {}

		explicit Coordinate( const float i_x, const float i_y, const float i_z ) noexcept : glm::vec3( i_x, i_y, i_z )
		{}

		explicit Coordinate( const float value ) : glm::vec3( value ) {}
	};

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > lhs, const Vector vector );

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > lhs, const Scale scale );

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > lhs, const NormalVector vector );

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > lhs, const Coordinate< CType > rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) + static_cast< glm::vec3 >( rhs ) );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > operator-( const Coordinate< CType > lhs, const Coordinate< CType > rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) - static_cast< glm::vec3 >( rhs ) );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > lhs, const glm::vec3 rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) + rhs );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > operator-( const Coordinate< CType > lhs, const glm::vec3 rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) - rhs );
	}

	using ModelCoordinate = Coordinate< CoordinateSpace::Model >;
	using WorldCoordinate = Coordinate< CoordinateSpace::World >;

	static_assert( sizeof( glm::vec3 ) == sizeof( ModelCoordinate ) );

} // namespace fgl::engine

namespace glm
{

	template < fgl::engine::CoordinateSpace CType >
	inline float dot( const fgl::engine::Coordinate< CType > coord, const fgl::engine::Coordinate< CType > other )
	{
		return dot( static_cast< vec3 >( coord ), static_cast< vec3 >( other ) );
	}

	inline float dot( const fgl::engine::WorldCoordinate coord, const vec3 other )
	{
		return dot( static_cast< vec3 >( coord ), other );
	}

	inline float distance(
		const fgl::engine::Coordinate< fgl::engine::CoordinateSpace::World > coord,
		const fgl::engine::Coordinate< fgl::engine::CoordinateSpace::World > other )
	{
		return distance( static_cast< vec3 >( coord ), static_cast< vec3 >( other ) );
	}

} // namespace glm
