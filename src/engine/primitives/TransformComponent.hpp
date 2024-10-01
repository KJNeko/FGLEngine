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

	template < MatrixType MType, CoordinateSpace CType >
	TransformComponent< EvolvedType< MType >() >
		operator*( Matrix< MType > matrix, TransformComponent< CType > transform )
	{
		// TODO: HACKY FIX! The performance for this is probably dogshit. So we need to probably make sure scale is never completely zero

		// if any of the values in scale is zero then we need to set it to something very small
		// otherwise the decompose function will fail
		/*
		if ( transform.scale.x == 0.0f ) transform.scale.x = constants::EPSILON;
		if ( transform.scale.y == 0.0f ) transform.scale.y = constants::EPSILON;
		if ( transform.scale.z == 0.0f ) transform.scale.z = constants::EPSILON;

		// Do the same for the matrix
		if ( matrix[ 0 ][ 0 ] == 0.0f ) matrix[ 0 ][ 0 ] = constants::EPSILON;
		if ( matrix[ 1 ][ 1 ] == 0.0f ) matrix[ 1 ][ 1 ] = constants::EPSILON;
		if ( matrix[ 2 ][ 2 ] == 0.0f ) matrix[ 2 ][ 2 ] = constants::EPSILON;
		*/

		auto combined_matrix { matrix * transform.mat() };

		if ( combined_matrix[ 0 ][ 0 ] == 0.0f ) combined_matrix[ 0 ][ 0 ] = constants::EPSILON;
		if ( combined_matrix[ 1 ][ 1 ] == 0.0f ) combined_matrix[ 1 ][ 1 ] = constants::EPSILON;
		if ( combined_matrix[ 2 ][ 2 ] == 0.0f ) combined_matrix[ 2 ][ 2 ] = constants::EPSILON;

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

		return { Coordinate< EvolvedType< MType >() >( translation ), Scale( scale ), Rotation( quat ) };
	}

	// A game object will be going from world to camera space
	using GameObjectTransform = TransformComponent< CoordinateSpace::World >;
	// using ModelTransform = TransformComponent< CoordinateSpace::Model >;
	using WorldTransform = TransformComponent< CoordinateSpace::World >;

} // namespace fgl::engine