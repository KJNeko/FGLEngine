//
// Created by kj16609 on 2/17/24.
//

#include "Rotation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "engine/math/taitBryanMatrix.hpp"

namespace fgl::engine
{

	//! Converts 3-axis rotation (euler) to a quaternion
	glm::quat toQuat( const float pitch, const float roll, const float yaw )
	{
		const glm::vec3 rotation { glm::vec3( pitch, roll, yaw ) * glm::vec3( 0.5f ) };
		const glm::vec3 rot_cos { glm::cos( rotation ) };
		const glm::vec3 rot_sin { glm::sin( rotation ) };

		auto extractFloats = []( const glm::vec3& vec ) -> std::tuple< const float&, const float&, const float >
		{ return std::make_tuple( vec.x, vec.y, vec.z ); };

		const auto& [ cp, cr, cy ] = extractFloats( rot_cos );
		const auto& [ sp, sr, sy ] = extractFloats( rot_sin );

		return { sr * cp * sy - cr * sp * sy,
			     cr * sp * cy + sr * cp * sy,
			     cr * cp * sy - sr * sp * cy,
			     cr * cp * cy + sr * sp * sy };
	}

	Rotation::Rotation() : glm::quat( 1.0f, 0.0f, 0.0f, 0.0f )
	{}

	Rotation::Rotation( const float pitch_r, const float roll_r, const float yaw_r ) :
	  glm::quat( glm::toQuat( taitBryanMatrix( pitch_r, roll_r, yaw_r ) ) )
	{}

	Rotation& Rotation::operator=( const Rotation& rotation )
	{
		glm::quat::operator=( rotation );
		return *this;
	}

	Rotation& Rotation::operator+=( const Rotation& rotation )
	{
		glm::quat::operator+=( rotation );
		return *this;
	}

	NormalVector Rotation::forward() const
	{
		return mat() * NormalVector( constants::WORLD_FORWARD );
	}

	NormalVector Rotation::right() const
	{
		return mat() * NormalVector( constants::WORLD_RIGHT );
	}

	NormalVector Rotation::up() const
	{
		return mat() * NormalVector( constants::WORLD_UP );
	}

	RotationMatrix Rotation::mat() const
	{
		return { glm::mat4_cast( *this ) };
	}

	Rotation Rotation::operator*( const Rotation rot ) const
	{
		return Rotation( glm::normalize( static_cast< glm::quat >( *this ) * static_cast< glm::quat >( rot ) ) );
	}

} // namespace fgl::engine
