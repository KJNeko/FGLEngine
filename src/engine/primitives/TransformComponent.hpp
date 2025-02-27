//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Scale.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "rotation/QuatRotation.hpp"
#include "rotation/UniversalRotation.hpp"

namespace fgl::engine
{
	template < MatrixType >
	class Matrix;

	template < CoordinateSpace CType >
	consteval MatrixType MatrixTransformType()
	{
		switch ( CType )
		{
			case CoordinateSpace::Model:
				return MatrixType::InvalidMatrix;
			case CoordinateSpace::World:
				return MatrixType::ModelToWorld;
			case CoordinateSpace::Camera:
				FGL_UNREACHABLE();
			case CoordinateSpace::Screen:
				FGL_UNREACHABLE();
			default:
				FGL_UNREACHABLE();
		}
		FGL_UNREACHABLE();
	};

	//TransformComponent is always in world space
	template < CoordinateSpace CType = CoordinateSpace::World >
	struct TransformComponent
	{
		Coordinate< CType > translation { constants::WORLD_CENTER };
		Scale scale { 1.0f, 1.0f, 1.0f };
		UniversalRotation rotation { constants::DEFAULT_ROTATION };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		[[nodiscard]] glm::mat4 mat4() const;

		[[nodiscard]] Matrix< MatrixTransformType< CType >() > mat() const;

		[[nodiscard]] NormalVector forward() const;

		[[nodiscard]] NormalVector backwards() const { return -forward(); }

		[[nodiscard]] NormalVector right() const;

		[[nodiscard]] NormalVector left() const { return -right(); }

		[[nodiscard]] NormalVector up() const;

		[[nodiscard]] NormalVector down() const { return -up(); }
	};

	template < CoordinateSpace CType, MatrixType MType >
	TransformComponent< CType > decompose( const glm::mat4 matrix )
	{
		glm::mat4 localMatrix = matrix;

		[[maybe_unused]] glm::vec4 perspective {};
		glm::vec3 scale {};
		glm::vec3 skew {};

		// Normalize the matrix
		if ( glm::epsilonEqual( localMatrix[ 3 ][ 3 ], 0.0f, glm::epsilon< float >() ) )
			throw std::runtime_error( "Failed to decompose matrix: determinant is zero" );

		// First, isolate perspective if it exists.
		if ( glm::epsilonNotEqual( localMatrix[ 0 ][ 3 ], 0.0f, glm::epsilon< float >() )
		     || glm::epsilonNotEqual( localMatrix[ 1 ][ 3 ], 0.0f, glm::epsilon< float >() )
		     || glm::epsilonNotEqual( localMatrix[ 2 ][ 3 ], 0.0f, glm::epsilon< float >() ) )
		{
			// Copy perspective elements
			perspective =
				glm::vec4( localMatrix[ 0 ][ 3 ], localMatrix[ 1 ][ 3 ], localMatrix[ 2 ][ 3 ], localMatrix[ 3 ][ 3 ] );

			// Clear perspective row
			localMatrix[ 0 ][ 3 ] = localMatrix[ 1 ][ 3 ] = localMatrix[ 2 ][ 3 ] = 0.0f;
			localMatrix[ 3 ][ 3 ] = 1.0f;
		}
		else
		{
			// No perspective transformation
			perspective = glm::vec4( 0, 0, 0, 1 );
		}

		// Extract translation
		const glm::vec3 translation { localMatrix[ 3 ] };
		localMatrix[ 3 ] = glm::vec4( 0, 0, 0, localMatrix[ 3 ].w );

		// Extract scale and skew
		std::array< glm::vec3, 3 > row {};
		for ( int i = 0; i < 3; i++ ) row[ i ] = glm::vec3( localMatrix[ i ] );

		// Compute X scale and normalize first row
		scale.x = glm::length( row[ 0 ] );
		row[ 0 ] = glm::normalize( row[ 0 ] );

		// Compute XY shear factor and make 2nd row orthogonal to 1st
		skew.x = glm::dot( row[ 0 ], row[ 1 ] );
		row[ 1 ] = glm::mix( row[ 1 ], row[ 1 ] - row[ 0 ] * skew.x, glm::epsilon< float >() );

		// Compute Y scale and normalize 2nd row
		scale.y = glm::length( row[ 1 ] );
		row[ 1 ] = glm::normalize( row[ 1 ] );
		skew.x /= scale.y;

		// Compute XZ and YZ shears and orthogonalize 3rd row
		skew.y = glm::dot( row[ 0 ], row[ 2 ] );
		row[ 2 ] = glm::mix( row[ 2 ], row[ 2 ] - row[ 0 ] * skew.y, glm::epsilon< float >() );
		skew.z = glm::dot( row[ 1 ], row[ 2 ] );
		row[ 2 ] = glm::mix( row[ 2 ], row[ 2 ] - row[ 1 ] * skew.z, glm::epsilon< float >() );

		// Compute Z scale and normalize 3rd row
		scale.z = glm::length( row[ 2 ] );
		row[ 2 ] = glm::normalize( row[ 2 ] );
		skew.y /= scale.z;
		skew.z /= scale.z;

		// At this point, the rotation matrix is orthogonal (no skew), so it's safe to calculate the quaternion.
		const glm::quat rotation { glm::quat_cast( glm::mat3( row[ 0 ], row[ 1 ], row[ 2 ] ) ) };

		return { Coordinate< CType >( translation ), Scale( scale ), QuatRotation( rotation ) };
	}

	// A game object will be going from world to camera space
	using GameObjectTransform = TransformComponent< CoordinateSpace::World >;
	// using ModelTransform = TransformComponent< CoordinateSpace::Model >;
	using WorldTransform = TransformComponent< CoordinateSpace::World >;

} // namespace fgl::engine