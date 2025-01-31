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

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "engine/primitives/matricies/RotationMatrix.hpp"
#include "engine/primitives/vectors/NormalVector.hpp"

namespace fgl::engine
{
	struct Rotation;
}

namespace fgl::engine
{

	/*

	TODO: Fix the singularity issue. Upon reaching +90y if x & z are both 0.0f then a singularity is entered and gimal locking occurs.

	The fix for this seems to be detecting the singularity and doing a different formula for quat -> euler converstions.

	There is also an issue with the editor in that it is impossible to reach beyond +90y because of how the math comes out.
	The X and Z rapidly swap from +/-

	This swapping effect also hapens on quaternions.online, So it might be that both our implementations have some sort of fix
	 */

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
		inline glm::quat internal_quat() const { return *this; }

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
