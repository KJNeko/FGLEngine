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
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/matricies/MatrixEvolvedTypes.hpp"

namespace fgl::engine
{

	class Vector;
	class NormalVector;

	template < CoordinateSpace CType >
	class Coordinate : protected glm::vec3
	{
	  public:

		Coordinate( const Coordinate& other ) noexcept = default;
		Coordinate( Coordinate&& other ) = default;

		Coordinate() noexcept : glm::vec3( constants::DEFAULT_VEC3 ) {}

		explicit Coordinate( const glm::vec3 position ) noexcept : glm::vec3( position ) {}

		explicit Coordinate( const float i_x, const float i_y, const float i_z ) noexcept : glm::vec3( i_x, i_y, i_z )
		{}

		explicit Coordinate( const float value ) : glm::vec3( value ) {}

		explicit Coordinate( const Vector vector );

		float& up() { return z; }

		float up() const { return z; }

		float& right() { return x; }

		float right() const { return x; }

		float& forward() { return y; }

		float forward() const { return y; }

		glm::vec3& vec() { return static_cast< glm::vec3& >( *this ); }

		const glm::vec3& vec() const { return static_cast< const glm::vec3& >( *this ); }

		Coordinate operator+( const Vector other ) const;
		Coordinate operator-( const Vector other ) const;
		Coordinate& operator+=( const Vector other );
		Coordinate& operator-=( const Vector other );

		Coordinate operator+( const NormalVector other ) const;
		Coordinate operator-( const NormalVector other ) const;

		Coordinate operator+( const Coordinate other ) const;
		Coordinate operator-( const Coordinate other ) const;

		Coordinate operator+( const glm::vec3 other ) const;
		Coordinate operator-( const glm::vec3 other ) const;

		Coordinate& operator=( const Coordinate& other ) = default;
		Coordinate& operator=( Coordinate&& other ) = default;
	};

	using ModelCoordinate = Coordinate< CoordinateSpace::Model >;
	using WorldCoordinate = Coordinate< CoordinateSpace::World >;

	static_assert( sizeof( glm::vec3 ) == sizeof( ModelCoordinate ) );

} // namespace fgl::engine