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

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

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

	struct Rotation : private glm::quat
	{
		Rotation();

		Rotation( float x, float y, float z );

		Rotation( float value );

		Rotation( const glm::quat& quat ) : glm::quat( quat ) {}

		RotationMatrix mat() const { return { glm::toMat3( *this ) }; }

		NormalVector forward() const;
		NormalVector right() const;
		NormalVector up() const;

		glm::vec3 euler() const;
		float xAngle() const;
		float yAngle() const;
		float zAngle() const;

		void setX( float );
		void setY( float );
		void setZ( float );

		// These will add a rotation using the local axis
		void addX( float );
		void addY( float );
		void addZ( float );

		// These will add a rotation using the world axis
		void addXWorld( float );
		void addYWorld( float );
		void addZWorld( float );

		// internal
		inline glm::quat internal_quat() const { return static_cast< glm::quat >( *this ); }

		bool operator==( const Rotation& rot ) const
		{
			return static_cast< glm::quat >( *this ) == static_cast< glm::quat >( rot );
		}

		friend bool operator==( const Rotation&, const glm::quat& );
	};

	inline bool operator==( const Rotation& rot, const glm::quat& quat )
	{
		return static_cast< glm::quat >( rot ) == quat;
	}

	namespace constants
	{
		constexpr glm::vec3 DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

} // namespace fgl::engine
