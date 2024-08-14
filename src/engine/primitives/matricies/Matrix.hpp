//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
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

		glm::mat3 rotmat() const { return glm::mat3( *this ); }

		// When extracting a quaternion from a matrix the matrix ***MUST*** be pure. No scale or translation is able to be in the same matrix
		glm::quat quat() const
		{
			// Here's to hoping the compiler will properly optimize this function to oblivion?
			[[maybe_unused]] glm::vec3 scale, translation, skew;
			glm::quat quat;
			[[maybe_unused]] glm::vec4 perspective;
			glm::decompose( *this, scale, quat, translation, skew, perspective );
			return quat;
		}

		explicit Matrix( const float i_value = 1.0f ) : glm::mat4( i_value ) {}

		explicit Matrix( const glm::mat4& matrix ) : glm::mat4( matrix ) {}

		Matrix operator*( const Matrix& other )
		{
			return Matrix( static_cast< glm::mat4 >( *this ) * static_cast< glm::mat4 >( other ) );
		}

		glm::mat4 inverse() const { return glm::inverse( *this ); }
	};

	//Lines
	template < CoordinateSpace CType, MatrixType MType >
		requires can_be_evolved< CType, MType >
	LineSegment< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const LineSegment< CType > );

	template < CoordinateSpace CType, MatrixType MType >
	InfiniteLine< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const InfiniteLine< CType > );

	//Planes
	template < CoordinateSpace CType, MatrixType MType >
		requires can_be_evolved< CType, MType >
	OriginDistancePlane< EvolvedType< MType >() >
		operator*( const Matrix< MType > mat, const OriginDistancePlane< CType > );

	template < CoordinateSpace CType, MatrixType MType >
		requires can_be_evolved< CType, MType >
	PointPlane< EvolvedType< MType >() > operator*( const Matrix< MType > mat, const PointPlane< CType > );

	//Coordinates
	template < CoordinateSpace CType, MatrixType MType >
		requires can_be_evolved< CType, MType >
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
