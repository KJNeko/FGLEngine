//
// Created by kj16609 on 2/28/24.
//

#pragma once

#include <glm/vec3.hpp>

#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/matricies/MatrixEvolvedTypes.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{

	class Vector;

	class NormalVector : public glm::vec3
	{
	  public:

		NormalVector( const Vector vec );
		explicit NormalVector( const glm::vec3 vec );

		Vector operator*( const float scalar );
	};

	template < MatrixType MType >
	NormalVector operator*( const Matrix< MType > mat, const NormalVector vector )
	{
		return NormalVector( mat * glm::vec4( static_cast< glm::vec3 >( vector ), 0.0f ) );
	}

	template < CoordinateSpace CType >
	Coordinate< CType > operator+( const Coordinate< CType > coord, const NormalVector normal )
	{
		return Coordinate< CType >( coord + static_cast< glm::vec3 >( normal ) );
	}

} // namespace fgl::engine
