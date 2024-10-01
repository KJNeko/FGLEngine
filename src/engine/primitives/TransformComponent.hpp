//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Rotation.hpp"
#include "Scale.hpp"
#include "engine/FGL_DEFINES.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/primitives/points/Coordinate.hpp"

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
		Rotation rotation { 0.0f, 0.0f, 0.0f };

		//TODO: Figure this out and replace TransformComponent with a template of CType instead
		glm::mat4 mat4() const;

		Matrix< MatrixTransformType< CType >() > mat() const;

		NormalVector forward() const;

		NormalVector backwards() const { return -forward(); }

		NormalVector right() const;

		NormalVector left() const { return -right(); }

		NormalVector up() const;

		NormalVector down() const { return -up(); }
	};

	template < CoordinateSpace CType, MatrixType MType >
	TransformComponent< CType > decompose( const glm::mat4 matrix )
	{
		glm::mat4 localMatrix = matrix;

		glm::vec4 perspective;
		glm::vec3 scale;
		glm::vec3 skew;

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
		glm::vec3 row[ 3 ];
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

		return { Coordinate< CType >( translation ), Scale( scale ), Rotation( rotation ) };
	}

	template < MatrixType MType, CoordinateSpace CType >
	TransformComponent< EvolvedType< MType >() >
		operator*( Matrix< MType > matrix, TransformComponent< CType > transform )
	{
		// TODO: HACKY FIX! The performance for this is probably dogshit. So we need to probably make sure scale is never completely zero

		// if any of the values in scale is zero then we need to set it to something very small
		// otherwise the decompose function will fail
		if ( transform.scale.x == 0.0f ) transform.scale.x = constants::EPSILON;
		if ( transform.scale.y == 0.0f ) transform.scale.y = constants::EPSILON;
		if ( transform.scale.z == 0.0f ) transform.scale.z = constants::EPSILON;

		// Do the same for the matrix
		if ( matrix[ 0 ][ 0 ] == 0.0f ) matrix[ 0 ][ 0 ] = constants::EPSILON;
		if ( matrix[ 1 ][ 1 ] == 0.0f ) matrix[ 1 ][ 1 ] = constants::EPSILON;
		if ( matrix[ 2 ][ 2 ] == 0.0f ) matrix[ 2 ][ 2 ] = constants::EPSILON;

		// For some reason GLM decompose doesn't seem to work with left handed coordinate systems. So we just made our own!
		// This probably is shittily written and likely full of bugs. But hey, It works for now
		// TODO: Verify with entry
		/*
		auto combined_matrix { matrix * transform.mat() };

		if ( combined_matrix[ 0 ][ 0 ] == 0.0f ) combined_matrix[ 0 ][ 0 ] = constants::EPSILON * 2;
		if ( combined_matrix[ 1 ][ 1 ] == 0.0f ) combined_matrix[ 1 ][ 1 ] = constants::EPSILON * 2;
		if ( combined_matrix[ 2 ][ 2 ] == 0.0f ) combined_matrix[ 2 ][ 2 ] = constants::EPSILON * 2;

		const auto NaN { std::numeric_limits< float >::quiet_NaN() };

		glm::vec3 scale { NaN }, translation { NaN };
		[[maybe_unused]] glm::vec3 skew { NaN };
		glm::quat quat { NaN, NaN, NaN, NaN };
		[[maybe_unused]] glm::vec4 perspective { NaN };

		if ( !glm::decompose( combined_matrix, scale, quat, translation, skew, perspective ) )
		{
			log::warn(
				"Failed to decompose matrix:\n{},\n{}",
				static_cast< glm::mat4 >( matrix ),
				static_cast< glm::mat4 >( combined_matrix ) );
		}

		assert( !std::isnan( scale.x ) );
		assert( scale != glm::vec3( 0.0f, 0.0f, 0.0f ) );
		*/

		auto combined_matrix { matrix * transform.mat() };

		return decompose< EvolvedType< MType >(), MType >( combined_matrix );
		// return { Coordinate< EvolvedType< MType >() >( translation ), Scale( scale ), Rotation( quat ) };
	}

	// A game object will be going from world to camera space
	using GameObjectTransform = TransformComponent< CoordinateSpace::World >;
	// using ModelTransform = TransformComponent< CoordinateSpace::Model >;
	using WorldTransform = TransformComponent< CoordinateSpace::World >;

} // namespace fgl::engine