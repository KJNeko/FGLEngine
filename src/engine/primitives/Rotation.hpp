//
// Created by kj16609 on 2/17/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#pragma GCC diagnostic pop

#include <numbers>
#include <utility>

#include "engine/primitives/matricies/RotationMatrix.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"
#include "engine/primitives/vectors/Vector.hpp"

namespace fgl::engine
{
	struct Rotation;
}

namespace fgl::engine
{
	enum class RotationModifierType
	{
		Pitch,
		Roll,
		Yaw
	};

	struct Rotation;

	template < RotationModifierType ModifierType >
	class RotationModifier
	{
		using enum RotationModifierType;

		Rotation& rot;

		friend struct Rotation;

		RotationModifier() = delete;

		RotationModifier( Rotation& i_rot ) : rot( i_rot ) {}

	  public:

		Rotation& operator+=( const float scalar );
		Rotation& operator-=( const float scalar );

		operator float() const;

		float value() const { return static_cast< float >( *this ); }

		float value() { return static_cast< float >( *this ); }
	};

	struct Rotation : private glm::quat
	{
		friend class RotationModifier< RotationModifierType::Pitch >;
		friend class RotationModifier< RotationModifierType::Roll >;
		friend class RotationModifier< RotationModifierType::Yaw >;

	  public:

		Rotation();

		explicit Rotation( const float pitch_r, const float roll_r, const float yaw_r );

		Rotation( const Rotation& other ) = default;

		Rotation( const glm::quat other ) : glm::quat( other ) {}

		FGL_FORCE_INLINE_FLATTEN auto pitch() { return RotationModifier< RotationModifierType::Pitch >( *this ); }

		FGL_FORCE_INLINE_FLATTEN auto roll() { return RotationModifier< RotationModifierType::Roll >( *this ); }

		FGL_FORCE_INLINE_FLATTEN auto yaw() { return RotationModifier< RotationModifierType::Yaw >( *this ); }

		FGL_FORCE_INLINE float pitch() const
		{
			//TODO: Ask entry to explain this stuff
			const float sinr_cosp { 2.0f * ( w * x + y * z ) };
			const float cosr_cosp { 1.0f - 2.0f * ( x * x + y * y ) };
			return std::atan2( sinr_cosp, cosr_cosp );
		}

		FGL_FORCE_INLINE float roll() const
		{
			const float sinp { glm::sqrt( 1.0f + 2.0f * ( w * y - x * z ) ) };
			const float cosp { glm::sqrt( 1.0f - 2.0f * ( w * y - x * z ) ) };
			return 2.0f * std::atan2( sinp, cosp ) - ( std::numbers::pi_v< float > / 2.0f );
		}

		FGL_FORCE_INLINE float yaw() const
		{
			const float siny_cosp { 2.0f * ( w * z + x * y ) };
			const float cosy_cosp { 1.0f - 2.0f * ( y * y + z * z ) };
			return std::atan2( siny_cosp, cosy_cosp );
		}

		Rotation& operator=( const Rotation other );

		Rotation& operator+=( const Rotation vec );

		NormalVector forward() const;

		NormalVector right() const;

		NormalVector up() const;

		RotationMatrix mat() const;

		Rotation operator*( const Rotation other ) const;

		bool operator==( const Rotation& other ) const = default;
	};

	template < RotationModifierType ModifierType >
	FGL_FORCE_INLINE_FLATTEN RotationModifier< ModifierType >::operator float() const
	{
		switch ( ModifierType )
		{
			case RotationModifierType::Pitch:
				return const_cast< const Rotation& >( rot ).pitch();
			case RotationModifierType::Roll:
				return const_cast< const Rotation& >( rot ).roll();
			case RotationModifierType::Yaw:
				return const_cast< const Rotation& >( rot ).yaw();
		}
		std::unreachable();
	}

	namespace constants
	{
		constexpr glm::vec3 DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

	template < RotationModifierType MT >
	consteval glm::vec3 getModifierAxis()
	{
		switch ( MT )
		{
			case RotationModifierType::Pitch:
				return constants::WORLD_RIGHT;
			case RotationModifierType::Roll:
				return constants::WORLD_FORWARD;
			case RotationModifierType::Yaw:
				return constants::WORLD_DOWN;
		}
	}

	template < RotationModifierType ModifierType >
	Rotation& RotationModifier< ModifierType >::operator+=( const float scalar )
	{
		rot = Rotation( static_cast< glm::quat >( rot ) * glm::angleAxis( scalar, getModifierAxis< ModifierType >() ) );
		return rot;
	}

	template < RotationModifierType ModifierType >
	Rotation& RotationModifier< ModifierType >::operator-=( const float scalar )
	{
		rot =
			Rotation( static_cast< glm::quat >( rot ) * glm::angleAxis( -scalar, getModifierAxis< ModifierType >() ) );
		return rot;
	}

} // namespace fgl::engine
