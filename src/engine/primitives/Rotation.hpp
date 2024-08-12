//
// Created by kj16609 on 2/17/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
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
		Pitch = 0,
		Roll = 1,
		Yaw = 2,
	};

	struct Rotation;

	template < RotationModifierType ModifierType, bool is_const = false >
	class RotationModifier;

	template < RotationModifierType ModifierType >
	using ConstRotationModifier = RotationModifier< ModifierType, true >;

	struct Rotation : public glm::quat
	{
		template < RotationModifierType ModifierType, bool is_const >
		friend class RotationModifier;

		Rotation();

		explicit Rotation( float pitch, float roll, float yaw );

		Rotation( const Rotation& other ) = default;

		explicit Rotation( const glm::quat other ) : glm::quat( glm::normalize( other ) ) {}

		explicit Rotation( const float scalar ) : Rotation( scalar, scalar, scalar ) {}

		enum RotationReference
		{
			Local,
			Global
		};

		RotationModifier< RotationModifierType::Pitch > pitch();
		ConstRotationModifier< RotationModifierType::Pitch > pitch() const;

		RotationModifier< RotationModifierType::Roll > roll();
		ConstRotationModifier< RotationModifierType::Roll > roll() const;

		RotationModifier< RotationModifierType::Yaw > yaw();
		ConstRotationModifier< RotationModifierType::Yaw > yaw() const;

		glm::vec3 euler() const;

		Rotation& operator=( const Rotation& rotation );
		Rotation& operator=( const glm::quat& rotation );

		Rotation& operator+=( const Rotation& rotation );

		NormalVector forward() const;

		NormalVector back() const { return -forward(); }

		NormalVector right() const;

		NormalVector left() const { return -right(); }

		NormalVector up() const;

		NormalVector down() const { return -up(); }

		RotationMatrix mat() const;

		Rotation operator*( const Rotation& other ) const;

		bool operator==( const Rotation& other ) const = default;
	};

	inline Rotation operator*( const glm::quat quat, const Rotation rotation )
	{
		return Rotation( quat * static_cast< glm::quat >( rotation ) );
	}

	template < RotationModifierType MT >
	glm::vec3 getModifierAxis()
	{
		switch ( MT )
		{
			case RotationModifierType::Pitch:
				return constants::WORLD_RIGHT;
			case RotationModifierType::Roll:
				return constants::WORLD_FORWARD;
			case RotationModifierType::Yaw:
				return -constants::WORLD_UP;
			default:
				FGL_UNREACHABLE();
		}

		FGL_UNREACHABLE();
	}

	template < RotationModifierType ModifierType, bool is_const >
	class RotationModifier
	{
		using enum RotationModifierType;

		using RotationType = std::conditional_t< is_const, const Rotation&, Rotation& >;

		RotationType rot;

		friend struct Rotation;

		RotationModifier() = delete;

		explicit RotationModifier( RotationType& i_rot ) : rot( i_rot ) {}

	  public:

		Rotation& operator+=( const float scalar );

		Rotation& operator-=( const float scalar );

		Rotation& operator=( const float scalar );

		operator float() const;

		float value() const;
	};

	namespace constants
	{
		constexpr glm::vec3 DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

} // namespace fgl::engine
