//
// Created by kj16609 on 2/7/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/glm.hpp>
#pragma GCC diagnostic pop

#include "engine/constants.hpp"
#include "engine/primitives/CoordinateSpace.hpp"
#include "engine/primitives/matricies/Matrix.hpp"

namespace fgl::engine
{
	class Vector;
	class NormalVector;

	template < CoordinateSpace CType >
	class Coordinate : private glm::vec3
	{
	  public:

		using glm::vec3::x;
		using glm::vec3::y;
		using glm::vec3::z;

		Coordinate( const Coordinate& other ) noexcept = default;
		Coordinate( Coordinate&& other ) = default;

		Coordinate() noexcept : glm::vec3( constants::DEFAULT_VEC3 ) {}

		explicit Coordinate( const glm::vec3 position ) noexcept : glm::vec3( position ) {}

		explicit Coordinate( const float i_x, const float i_y, const float i_z ) noexcept : glm::vec3( i_x, i_y, i_z )
		{}

		explicit Coordinate( const float value ) : glm::vec3( value ) {}

		explicit Coordinate( const Vector vector );

		inline float& up() { return z; }

		inline float up() const { return z; }

		inline float& right() { return x; }

		inline float right() const { return x; }

		inline float& forward() { return y; }

		inline float forward() const { return y; }

		inline glm::vec3& vec() { return static_cast< glm::vec3& >( *this ); }

		inline const glm::vec3& vec() const { return static_cast< const glm::vec3& >( *this ); }

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

		bool operator==( const Coordinate& other ) const = default;

		NormalVector normalTo( const Coordinate& target ) const;
		Vector vectorTo( const Coordinate& target ) const;
	};

	using ModelCoordinate = Coordinate< CoordinateSpace::Model >;
	using WorldCoordinate = Coordinate< CoordinateSpace::World >;

	static_assert( sizeof( glm::vec3 ) == sizeof( ModelCoordinate ) );

	template < CoordinateSpace CType >
	inline double distance( const Coordinate< CType >& p1, const Coordinate< CType >& p2 )
	{
		return length( p1.vec() - p2.vec() );
	}

	template < engine::CoordinateSpace CType >
	inline engine::Coordinate< CType >
		midpoint( const engine::Coordinate< CType > left, const engine::Coordinate< CType > right )
	{
		const auto x { ( left.vec().x + right.vec().x ) / 2.0f };
		const auto y { ( left.vec().y + right.vec().y ) / 2.0f };
		const auto z { ( left.vec().z + right.vec().z ) / 2.0f };

		return Coordinate< CType >( x, y, z );
	}

} // namespace fgl::engine
