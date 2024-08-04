//
// Created by kj16609 on 2/28/24.
//

#include "NormalVector.hpp"

#include "Vector.hpp"

namespace fgl::engine
{

	NormalVector::NormalVector( const glm::vec3 vector ) : glm::vec3( glm::normalize( vector ) )
	{}

	NormalVector::NormalVector( const fgl::engine::Vector vector ) : NormalVector( vector.vec() )
	{}

	Vector NormalVector::operator*( const float scalar ) const
	{
		return Vector( static_cast< glm::vec3 >( *this ) * scalar );
	}

} // namespace fgl::engine
