//
// Created by kj16609 on 2/17/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/glm/glm.hpp>
#include <glm/glm/gtc/quaternion.hpp>
#pragma GCC diagnostic pop

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/fwd.hpp>
#include <glm/glm/gtx/quaternion.hpp>

#include "engine/primitives/matricies/RotationMatrix.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{
	struct QuatRotation;
}

namespace fgl::engine
{
	struct QuatRotation : private glm::quat
	{
		QuatRotation();

		QuatRotation( float x, float y, float z );

		QuatRotation( float value );

		QuatRotation( const glm::quat& quat ) : glm::quat( quat ) {}

		RotationMatrix mat() const { return { glm::toMat3( *this ) }; }

		NormalVector forward() const;
		NormalVector right() const;
		NormalVector up() const;

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
		inline glm::quat internal_quat() const { return *this; }

		bool operator==( const QuatRotation& rot ) const
		{
			return static_cast< glm::quat >( *this ) == static_cast< glm::quat >( rot );
		}

		friend bool operator==( const QuatRotation&, const glm::quat& );
	};

	inline bool operator==( const QuatRotation& rot, const glm::quat& quat )
	{
		return static_cast< glm::quat >( rot ) == quat;
	}

	namespace constants
	{
		constexpr glm::vec3 DEFAULT_ROTATION { 0.0f, 0.0f, 0.0f };
	}

} // namespace fgl::engine
