//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include "Coordinate.hpp"

namespace fgl::engine
{
	enum class MatrixType
	{
		ModelToWorld,

		WorldToCamera,
		CameraToScreen,

		//WorldToScreen is two combined matricies (WorldToView and ViewToScreen)
		WorldToScreen
	};

	template < MatrixType MType >
	class Matrix : public glm::mat4
	{
	  public:

		explicit Matrix( const float value = 1.0f ) : glm::mat4( value ) {}

		explicit Matrix( const glm::mat4& matrix ) : glm::mat4( matrix ) {}
	};

	inline Matrix< MatrixType::WorldToScreen >
		operator*( const Matrix< MatrixType::CameraToScreen >& lhs, const Matrix< MatrixType::WorldToCamera >& rhs )
	{
		return Matrix< MatrixType::WorldToScreen >( static_cast< glm::mat4 >( lhs ) * static_cast< glm::mat4 >( rhs ) );
	}

	template < MatrixType MType >
	consteval CoordinateSpace EvolvedType();

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::ModelToWorld >()
	{
		return CoordinateSpace::World;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::WorldToCamera >()
	{
		return CoordinateSpace::Camera;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::CameraToScreen >()
	{
		return CoordinateSpace::Screen;
	}

	template <>
	consteval CoordinateSpace EvolvedType< MatrixType::WorldToScreen >()
	{
		return CoordinateSpace::Screen;
	}

	//Operators
	template < MatrixType MType >
	inline Coordinate< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const Coordinate< EvolvedType< MType >() >& coordinate )
	{
		return Coordinate< EvolvedType< MType >() >( matrix * glm::vec4( coordinate, 1.0f ) );
	}

}; // namespace fgl::engine
