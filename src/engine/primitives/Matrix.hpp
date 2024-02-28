//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <glm/mat4x4.hpp>

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
	struct Matrix : public glm::mat4
	{
		explicit Matrix( const float i_value = 1.0f ) : glm::mat4( i_value ) {}

		explicit Matrix( const glm::mat4& matrix ) : glm::mat4( matrix ) {}
	};

	inline Matrix< MatrixType::WorldToScreen >
		operator*( const Matrix< MatrixType::CameraToScreen >& lhs, const Matrix< MatrixType::WorldToCamera >& rhs )
	{
		return Matrix< MatrixType::WorldToScreen >( static_cast< glm::mat4 >( lhs ) * static_cast< glm::mat4 >( rhs ) );
	}

}; // namespace fgl::engine
