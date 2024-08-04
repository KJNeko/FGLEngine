//
// Created by kj16609 on 2/17/24.
//

#include "Rotation.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "engine/debug/drawers.hpp"

namespace fgl::engine
{

	Rotation::Rotation() : glm::quat( 1.0f, 0.0f, 0.0f, 0.0f )
	{}

	inline glm::quat buildQuat( const glm::vec3 euler )
	{
		// euler should be in `pitch, roll, yaw` order
		const auto pitch { glm::angleAxis( euler.x, constants::WORLD_RIGHT ) };
		const auto roll { glm::angleAxis( euler.y, constants::WORLD_FORWARD ) };
		const auto yaw { glm::angleAxis( euler.z, constants::WORLD_UP ) };

		return pitch * roll * yaw;
	}

	inline float pitch( const glm::quat& q )
	{
		const float y { 2.0f * ( q.y * q.z + q.w * q.x ) };
		const float x { q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z };

		if ( glm::all( glm::equal( glm::vec2( x, y ), glm::vec2( 0.0f ), glm::epsilon< float >() ) ) ) return 0.0f;

		return glm::atan( y, x );
	}

	inline float roll( const glm::quat& q )
	{
		return std::asin( glm::clamp( -2.0f * ( q.x * q.z - q.w * q.y ), -1.0f, 1.0f ) );
	}

	inline float yaw( const glm::quat& q )
	{
		const float y { 2.0f * ( q.x * q.y + q.w * q.z ) };
		const float x { q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z };

		if ( glm::all( glm::equal( glm::vec2( x, y ), glm::vec2( 0.0f ), glm::epsilon< float >() ) ) ) return 0.0f;

		return glm::atan( y, x );
	}

	// Because of how glm does stuff. We need to invert the roll.
	Rotation::Rotation( const float pitch, const float roll, const float yaw ) :
	  glm::quat( buildQuat( { pitch, roll, yaw } ) )
	{
		FGL_ASSERT( ::fgl::engine::pitch( *this ) - pitch < glm::epsilon< float >() );
		FGL_ASSERT( ::fgl::engine::yaw( *this ) - yaw < glm::epsilon< float >() );
		FGL_ASSERT( ::fgl::engine::roll( *this ) - roll  < glm::epsilon< float >() );
	}

	RotationModifier< RotationModifierType::Roll > Rotation::roll()
	{
		return RotationModifier< RotationModifierType::Roll >( *this );
	}

	ConstRotationModifier< RotationModifierType::Pitch > Rotation::pitch() const
	{
		return ConstRotationModifier< RotationModifierType::Pitch >( *this );
	}

	RotationModifier< RotationModifierType::Pitch > Rotation::pitch()
	{
		return RotationModifier< RotationModifierType::Pitch >( *this );
	}

	ConstRotationModifier< RotationModifierType::Roll > Rotation::roll() const
	{
		return ConstRotationModifier< RotationModifierType::Roll >( *this );
	}

	RotationModifier< RotationModifierType::Yaw > Rotation::yaw()
	{
		return RotationModifier< RotationModifierType::Yaw >( *this );
	}

	ConstRotationModifier< RotationModifierType::Yaw > Rotation::yaw() const
	{
		return ConstRotationModifier< RotationModifierType::Yaw >( *this );
	}

	Rotation& Rotation::operator=( const Rotation& rotation )
	{
		glm::quat::operator=( rotation );
		return *this;
	}

	Rotation& Rotation::operator=( const glm::quat& rotation )
	{
		glm::quat::operator=( rotation );
		return *this;
	}

	Rotation& Rotation::operator+=( const Rotation& rotation )
	{
		glm::quat::operator+=( rotation );

		*this = glm::normalize( *this );

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
		return { glm::mat3_cast( *this ) };
	}

	Rotation Rotation::operator*( const Rotation& other ) const
	{
		return Rotation( glm::normalize( static_cast< glm::quat >( *this ) * static_cast< glm::quat >( other ) ) );
	}

} // namespace fgl::engine
