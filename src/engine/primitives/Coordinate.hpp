//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include <glm/glm.hpp>

#include <ostream>

#include "engine/constants.hpp"

namespace fgl::engine
{

	enum class CoordinateSpace
	{
		Model,
		World,
		Camera,
		Screen
	};

	class Vector;

	template < CoordinateSpace type >
	class Coordinate : public glm::vec3
	{
		using ValueT = float;

	  public:

		float& up() { return z; }

		float& right() { return x; }

		float& forward() { return y; }

		Coordinate() : glm::vec3( constants::DEFAULT_VEC3 ) {}

		constexpr explicit Coordinate( const glm::vec3 position ) : glm::vec3( position ) {}

		constexpr explicit Coordinate( const float x, const float y, const float z ) : glm::vec3( x, y, z ) {}

		constexpr explicit Coordinate( const float value ) : glm::vec3( value ) {}

		operator glm::vec4() const { return glm::vec4( x, y, z, 1.0f ); }

		Coordinate operator+( const glm::vec3 other )
		{
			assert( static_cast< glm::vec3 >( *this ) != constants::DEFAULT_VEC3 );
			assert( other != constants::DEFAULT_VEC3 );
			return Coordinate( static_cast< glm::vec3 >( *this ) + static_cast< glm::vec3 >( other ) );
		}

		Coordinate operator-( const glm::vec3 other )
		{
			assert( static_cast< glm::vec3 >( *this ) != constants::DEFAULT_VEC3 );
			assert( other != constants::DEFAULT_VEC3 );
			return Coordinate( static_cast< glm::vec3 >( *this ) - static_cast< glm::vec3 >( other ) );
		}

		Coordinate& operator=( const glm::vec3 other )
		{
			glm::vec3::operator=( other );
			return *this;
		}
	};

	using ModelCoordinate = Coordinate< CoordinateSpace::Model >;
	using WorldCoordinate = Coordinate< CoordinateSpace::World >;

	static_assert( sizeof( glm::vec3 ) == sizeof( ModelCoordinate ) );

	template < CoordinateSpace CType >
	inline ::std::ostream& operator<<( ::std::ostream& os, const Coordinate< CType > coord )
	{
		os << "(" << coord.x << ", " << coord.y << ", " << coord.z << ")";
		return os;
	}

} // namespace fgl::engine

namespace glm
{

	template < fgl::engine::CoordinateSpace CType >
	inline double dot( const fgl::engine::Coordinate< CType > coord, const fgl::engine::Coordinate< CType > other )
	{
		return glm::dot( static_cast< glm::vec3 >( coord ), static_cast< glm::vec3 >( other ) );
	}

	inline double dot( const fgl::engine::WorldCoordinate coord, const glm::vec3 other )
	{
		return glm::dot( static_cast< glm::vec3 >( coord ), other );
	}

	using namespace fgl::engine;

	template < CoordinateSpace CType >
	inline Coordinate< CType > operator+( const Coordinate< CType >& lhs, const Coordinate< CType >& rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) + static_cast< glm::vec3 >( rhs ) );
	}

	template < CoordinateSpace CType >
	inline Coordinate< CType > operator-( const Coordinate< CType >& lhs, const Coordinate< CType >& rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) - static_cast< glm::vec3 >( rhs ) );
	}

} // namespace glm
