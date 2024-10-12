//
// Created by kj16609 on 2/17/24.
//

#include "Rotation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stdexcept>

#include "engine/FGL_DEFINES.hpp"
#include "engine/constants.hpp"

namespace fgl::engine
{
	Rotation::Rotation() : Rotation( 0.0f )
	{}

	glm::quat toQuat( const float roll, const float pitch, const float yaw )
	{
		/*
		const glm::vec3 euler { x / 2.0f, y / 2.0f, z / 2.0f };
		const glm::vec3 sin { glm::sin( euler ) };
		const glm::vec3 cos { glm::cos( euler ) };

		glm::quat q {};
		q.w = cos.x * cos.y * cos.z + sin.x * sin.y * sin.z;
		q.x = sin.x * cos.y * cos.z - cos.x * sin.y * sin.z;
		q.y = cos.x * sin.y * cos.z + sin.x * cos.y * sin.z;
		q.z = cos.x * cos.y * sin.z - sin.x * sin.y * cos.z;
		*/

		const glm::quat q_x { glm::angleAxis( roll, constants::WORLD_X ) }; // Roll
		const glm::quat q_y { glm::angleAxis( -pitch, constants::WORLD_Y ) }; // Pitch
		// In order to get it so that PITCH+ is UP we must invert the pitch
		const glm::quat q_z { glm::angleAxis( yaw, constants::WORLD_Z ) }; // Yaw

		const glm::quat q { q_z * q_y * q_x };

		return q;
	}

	Rotation::Rotation( const float x_i, const float y_i, const float z_i ) : glm::quat( toQuat( x_i, y_i, z_i ) )
	{}

	Rotation::Rotation( const float value ) : Rotation( value, value, value )
	{}

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

	glm::vec3 Rotation::euler() const
	{
		return { xAngle(), yAngle(), zAngle() };
	}

	float Rotation::xAngle() const
	{
		//Extract X angle from quaternion
		const float sinr_cosp { 2.0f * ( w * x + y * z ) };
		const float cosr_cosp { 1.0f - 2.0f * ( x * x + y * y ) };
		return std::atan2( sinr_cosp, cosr_cosp );
	}

	float Rotation::yAngle() const
	{
		// Extract Y angle from quaternion
		const float sinp { std::sqrt( 1.0f + 2.0f * ( w * y - x * z ) ) };
		const float cosp { std::sqrt( 1.0f - 2.0f * ( w * y - x * z ) ) };

		// We must invert the pitch in order to 'fix' it after being flipped in the constructor and add functions
		return -( 2.0f * std::atan2( sinp, cosp ) - std::numbers::pi_v< float > / 2.0f );
	}

	float Rotation::zAngle() const
	{
		// Extract Z angle from quaternion
		const float siny_cosp { 2.0f * ( w * z + x * y ) };
		const float cosy_cosp { 1.0f - 2.0f * ( y * y + z * z ) };
		return std::atan2( siny_cosp, cosy_cosp );
	}

	void Rotation::setX( [[maybe_unused]] const float value )
	{
		FGL_UNIMPLEMENTED();
	}

	void Rotation::setY( [[maybe_unused]] const float value )
	{
		FGL_UNIMPLEMENTED();
	}

	void Rotation::setZ( [[maybe_unused]] const float value )
	{
		FGL_UNIMPLEMENTED();
	}

	void Rotation::addX( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_X ) };
		*this = *this * q;
	}

	void Rotation::addY( const float value )
	{
		// Because the camera is flipped. We must also flip the pitch rotation
		const glm::quat q { glm::angleAxis( -value, constants::WORLD_Y ) };
		*this = *this * q;
	}

	void Rotation::addZ( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Z ) };
		*this = *this * q;
	}

	void Rotation::addXWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_X ) };
		*this = q * *this;
	}

	void Rotation::addYWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Y ) };
		*this = q * *this;
	}

	void Rotation::addZWorld( const float value )
	{
		const glm::quat q { glm::angleAxis( value, constants::WORLD_Z ) };
		*this = q * *this;
	}

} // namespace fgl::engine
