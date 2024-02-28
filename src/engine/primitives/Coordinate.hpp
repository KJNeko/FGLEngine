//
// Created by kj16609 on 2/7/24.
//

#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>

#include <ostream>

#include "CoordinateSpace.hpp"
#include "Matrix.hpp"
#include "MatrixEvolvedTypes.hpp"
#include "engine/constants.hpp"

namespace fgl::engine
{

	class Vector;

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

		Coordinate& operator=( const glm::vec3 other )
		{
			glm::vec3::operator=( other );
			return *this;
		}
	};

#ifndef NDEBUG
	template < CoordinateSpace CType >
	bool operator==( const Coordinate< CType > lhs, const Coordinate< CType >& rhs )
	{
		return static_cast< glm::vec3 >( lhs ) == static_cast< glm::vec3 >( rhs );
	}

	template < CoordinateSpace CType >
	bool operator==( const Coordinate< CType > lhs, const glm::vec3 rhs )
	{
		const auto diff { glm::abs( static_cast< glm::vec3 >( lhs ) - rhs ) };
		return glm::all( glm::lessThanEqual( diff, glm::vec3( constants::EPSILON ) ) );
		//These should have the same behaviour. I'm kind of confused why they don't?
		//return glm::all( glm::epsilonEqual( static_cast< glm::vec3 >( *this ), other, constants::EPSILON ) );
	}
#endif

	template < CoordinateSpace type >
	Coordinate< type > operator-( const Coordinate< type >& lhs, const Coordinate< type >& rhs )
	{
		return Coordinate< type >( static_cast< glm::vec3 >( lhs ) - static_cast< glm::vec3 >( rhs ) );
	}

	template < CoordinateSpace type >
	Coordinate< type > operator+( const Coordinate< type >& lhs, const Coordinate< type >& rhs )
	{
		return Coordinate< type >( static_cast< glm::vec3 >( lhs ) + static_cast< glm::vec3 >( rhs ) );
	}

	template < CoordinateSpace CType, MatrixType MType >
	Coordinate< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const Coordinate< CType > coord )
	{
		return Coordinate< EvolvedType<
			MType >() >( static_cast< glm::mat4 >( mat ) * glm::vec4( static_cast< glm::vec3 >( coord ), 1.0f ) );
	}

	using ModelCoordinate = Coordinate< CoordinateSpace::Model >;
	using WorldCoordinate = Coordinate< CoordinateSpace::World >;

	static_assert( sizeof( glm::vec3 ) == sizeof( ModelCoordinate ) );

	template < CoordinateSpace CType >
	::std::ostream& operator<<( ::std::ostream& os, const Coordinate< CType > coord )
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
		return dot( static_cast< vec3 >( coord ), static_cast< vec3 >( other ) );
	}

	inline double dot( const fgl::engine::WorldCoordinate coord, const vec3 other )
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
