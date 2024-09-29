//
// Created by kj16609 on 2/15/24.
//

#pragma once

#include "Rotation.hpp"
#include "Scale.hpp"
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
		operator*( const Matrix< MType >& matrix, const TransformComponent< CType >& transform )
	{
		const auto combined_matrix { matrix * transform.mat() };

		glm::vec3 scale {}, translation {};
		[[maybe_unused]] glm::vec3 skew {};
		glm::quat quat {};
		[[maybe_unused]] glm::vec4 perspective {};
		glm::decompose( combined_matrix, scale, quat, translation, skew, perspective );

		return { Coordinate< EvolvedType< MType >() >( translation ), Scale( scale ), Rotation( quat ) };
	}

	// A game object will be going from world to camera space
	using GameObjectTransform = TransformComponent< CoordinateSpace::World >;
	// using ModelTransform = TransformComponent< CoordinateSpace::Model >;
	using WorldTransform = TransformComponent< CoordinateSpace::World >;

} // namespace fgl::engine
