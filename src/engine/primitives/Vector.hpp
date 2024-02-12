//
// Created by kj16609 on 2/11/24.
//

#pragma once

namespace fgl::engine
{

	template < CoordinateSpace type >
	class Coordinate;

	class Vector : public glm::vec3
	{
	  public:

		explicit Vector( const glm::vec3 direction ) : glm::vec3( direction ) {}

		operator glm::vec4() const { return glm::vec4( static_cast< glm::vec3 >( *this ), 0.0f ); }

		Vector norm() const { return Vector( glm::normalize( static_cast< glm::vec3 >( *this ) ) ); }

		Vector operator*( const float scalar ) const { return Vector( static_cast< glm::vec3 >( *this ) * scalar ); }
	};

} // namespace fgl::engine

namespace glm
{

	inline glm::vec3 normalize( fgl::engine::Vector vector )
	{
		return glm::normalize( static_cast< glm::vec3 >( vector ) );
	}

	inline glm::vec3 cross( const fgl::engine::Vector vec, const glm::vec3 other )
	{
		return glm::cross( static_cast< glm::vec3 >( vec ), other );
	}

	template < CoordinateSpace CType >
	inline Coordinate< CType > operator+( const Coordinate< CType > lhs, const Vector rhs )
	{
		return Coordinate< CType >( static_cast< glm::vec3 >( lhs ) + static_cast< glm::vec3 >( rhs ) );
	}

} // namespace glm
