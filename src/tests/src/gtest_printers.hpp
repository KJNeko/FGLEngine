//
// Created by kj16609 on 2/15/24.
//

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <catch2/catch_all.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/primitives/Scale.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "engine/primitives/rotation/EulerRotation.hpp"
#include "engine/primitives/rotation/QuatRotation.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace Catch
{
	template <>
	struct StringMaker< glm::vec3 >
	{
		static std::string convert( const glm::vec3& vec ) { return glm::to_string( vec ); }
	};

	template <>
	struct StringMaker< fgl::engine::Vector >
	{
		static std::string convert( const fgl::engine::Vector& vec )
		{
			return StringMaker< glm::vec3 >::convert( static_cast< glm::vec3 >( vec.vec() ) );
		}
	};

	template <>
	struct StringMaker< glm::mat4 >
	{
		static std::string convert( const glm::mat4& mat )
		{
			return "\n\t" + glm::to_string( mat[ 0 ] ) + "\n\t" + glm::to_string( mat[ 1 ] ) + "\n\t"
			     + glm::to_string( mat[ 2 ] ) + "\n\t" + glm::to_string( mat[ 3 ] );
		}
	};

	template <>
	struct StringMaker< glm::quat >
	{
		static std::string convert( const glm::quat& quat )
		{
			return std::format( "({},{},{},{})", quat.w, quat.x, quat.y, quat.z );
		}
	};

	template <>
	struct StringMaker< fgl::engine::QuatRotation >
	{
		static std::string convert( const fgl::engine::QuatRotation& rot )
		{
			return StringMaker< glm::quat >::convert( rot.internal_quat() );
		}
	};

	template <>
	struct StringMaker< fgl::engine::EulerRotation >
	{
		static std::string convert( const fgl::engine::EulerRotation& rot )
		{
			return StringMaker< glm::vec3 >::convert( glm::vec3( rot.x, rot.y, rot.z ) );
		}
	};

	template < fgl::engine::MatrixType MType >
	struct StringMaker< fgl::engine::Matrix< MType > >
	{
		static std::string convert( const fgl::engine::Matrix< MType >& mat )
		{
			return StringMaker< glm::mat4 >::convert( static_cast< glm::mat4 >( mat ) );
		}
	};

	template <>
	struct StringMaker< ::fgl::engine::NormalVector >
	{
		static std::string convert( const fgl::engine::NormalVector vec )
		{
			return StringMaker< fgl::engine::Vector >::convert( static_cast< fgl::engine::Vector >( vec ) );
		}
	};

	template <>
	struct StringMaker< ::fgl::engine::Scale >
	{
		static std::string convert( const fgl::engine::Scale scale )
		{
			return StringMaker< glm::vec3 >::convert( static_cast< glm::vec3 >( scale ) );
		}
	};

	template <>
	struct StringMaker< ::fgl::engine::ModelCoordinate >
	{
		static std::string convert( const fgl::engine::ModelCoordinate coord )
		{
			return StringMaker< glm::vec3 >::convert( coord.vec() );
		}
	};

} // namespace Catch

#ifndef NDEBUG
namespace glm
{
	inline bool operator==( const glm::vec3& lhs, const glm::vec3& rhs )
	{
		return glm::all( ::glm::epsilonEqual( lhs, rhs, std::numeric_limits< float >::epsilon() ) );
	}

} // namespace glm
#else
#warning "Debug mode not enabled. Tests will fail when checking for floating point equality."
#endif
