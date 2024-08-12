//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/mat4x4.hpp>

#include "MatrixEvolvedTypes.hpp"

namespace fgl::engine
{
	//Lines
	template < CoordinateSpace CType >
	class LineSegment;
	template < CoordinateSpace CType >
	class InfiniteLine;

	//Planes
	template < CoordinateSpace CType >
	class OriginDistancePlane;
	template < CoordinateSpace CType >
	class PointPlane;

	//Coordinates
	template < CoordinateSpace CType >
	class Coordinate;

	//Vectors
	class Vector;
	struct NormalVector;

	template < MatrixType MType >
	class Matrix : public glm::mat4
	{
	  public:

		explicit Matrix( const float i_value = 1.0f ) : glm::mat4( i_value ) {}

		explicit Matrix( const glm::mat4& matrix ) : glm::mat4( matrix ) {}
	};

	//Lines
	template < CoordinateSpace CType, MatrixType MType >
	LineSegment< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const LineSegment< CType > );

	template < CoordinateSpace CType, MatrixType MType >
	InfiniteLine< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const InfiniteLine< CType > );

	//Planes
	template < CoordinateSpace CType, MatrixType MType >
	OriginDistancePlane< EvolvedType< MType >() >
		operator*( const Matrix< MType > mat, const OriginDistancePlane< CType > );

	template < CoordinateSpace CType, MatrixType MType >
	PointPlane< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const PointPlane< CType > );

	//Coordinates
	template < CoordinateSpace CType, MatrixType MType >
	Coordinate< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const Coordinate< CType > );

	//Vector
	template < MatrixType MType >
	Vector operator*( const Matrix< MType > mat, const Vector vec );

	template < MatrixType MType >
	NormalVector operator*( const Matrix< MType > mat, const NormalVector );

	inline Matrix< MatrixType::WorldToScreen >
		operator*( const Matrix< MatrixType::CameraToScreen > lhs, const Matrix< MatrixType::WorldToCamera > rhs )
	{
		return Matrix< MatrixType::WorldToScreen >( static_cast< glm::mat4 >( lhs ) * static_cast< glm::mat4 >( rhs ) );
	}

}; // namespace fgl::engine
