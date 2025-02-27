//
// Created by kj16609 on 2/17/24.
//

#include "QuatRotation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm/gtx/euler_angles.hpp>
#include <glm/glm/gtx/quaternion.hpp>

#include "constants.hpp"

namespace fgl::engine
{


	QuatRotation::QuatRotation() : QuatRotation( 0.0f )
	{}

	glm::quat toQuat( const float roll, const float pitch, const float yaw )
	{
		static_assert( 0.0f / 2.0f == 0.0f );

		const glm::vec3 euler { roll / 2.0f, -pitch / 2.0f, yaw / 2.0f };
		const glm::vec3 sin { glm::sin( euler ) };
		const glm::vec3 cos { glm::cos( euler ) };

		//TODO: There might be a better way to do this from euler angles. Since this is probably slow?
		glm::quat q {};
		q.w = cos.x * cos.y * cos.z + sin.x * sin.y * sin.z;
		q.x = sin.x * cos.y * cos.z - cos.x * sin.y * sin.z;
		q.y = cos.x * sin.y * cos.z + sin.x * cos.y * sin.z;
		q.z = cos.x * cos.y * sin.z - sin.x * sin.y * cos.z;

		/*
		const glm::quat q_x { glm::angleAxis( roll, constants::WORLD_X ) }; // Roll
		const glm::quat q_y { glm::angleAxis( -pitch, constants::WORLD_Y ) }; // Pitch
		// In order to get it so that PITCH+ is UP we must invert the pitch
		const glm::quat q_z { glm::angleAxis( yaw, constants::WORLD_Z ) }; // Yaw

		const glm::quat q { q_z * q_y * q_x };
		*/

		return glm::normalize( q );
	}

	QuatRotation::QuatRotation( const float x_i, const float y_i, const float z_i ) : glm::quat( toQuat( x_i, y_i, z_i ) )
	{}

	QuatRotation::QuatRotation( const float value ) : QuatRotation( value, value, value )
	{}

	NormalVector QuatRotation::forward() const
	{
		return mat() * NormalVector( constants::WORLD_FORWARD );
	}

	NormalVector QuatRotation::right() const
	{
		return mat() * NormalVector( constants::WORLD_RIGHT );
	}

	NormalVector QuatRotation::up() const
	{
		return mat() * NormalVector( constants::WORLD_UP );
	}

	void QuatRotation::addX( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_X ) };
		*this = *this * q;
	}

	void QuatRotation::addY( const float value )
	{
		// Because the camera is flipped. We must also flip the pitch rotation
		const glm::quat q { glm::angleAxis( -value, constants::WORLD_Y ) };
		*this = *this * q;
	}

	void QuatRotation::addZ( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Z ) };
		*this = *this * q;
	}

	void QuatRotation::addXWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_X ) };
		*this = q * *this;
	}

	void QuatRotation::addYWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Y ) };
		*this = q * *this;
	}

	void QuatRotation::addZWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Z ) };
		*this = q * *this;
	}

} // namespace fgl::engine
