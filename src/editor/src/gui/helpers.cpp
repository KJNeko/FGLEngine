//
// Created by kj16609 on 1/30/25.
//

#include "helpers.hpp"

#include "primitives/Rotation.hpp"

namespace fgl::engine::gui
{

	float clampEuler( const float value )
	{
		if ( value >= 180.0f )
		{
			// Wrap around.
			return value - 360.0f;
		}
		else if ( value <= -180.0f )
		{
			return value + 360.0f;
		}
		return value;
	}

	void dragFloat3Rot( const char* const label, Rotation& rot )
	{
		enum Axis
		{
			Pitch = 0,
			Yaw = 1,
			Roll = 2
		};

		const glm::vec3 c_dat { rot.euler() };
		glm::vec3 dat { glm::degrees( c_dat ) };

		ImGui::DragFloat3( label, &dat.x, 1.0f );

		dat = glm::radians( dat );

		const glm::vec3 diff { c_dat - dat };
		constexpr float epsilon { std::numeric_limits< float >::epsilon() };

		const glm::vec< 3, bool > changed_high { glm::greaterThanEqual( diff, glm::vec3( epsilon ) ) };
		const glm::vec< 3, bool > changed_low { glm::lessThanEqual( diff, glm::vec3( -epsilon ) ) };
		const glm::vec< 3, bool > changed { changed_high || changed_low };

		if ( changed[ Pitch ] )
		{
			dat[ Pitch ] = clampEuler( dat[ Pitch ] );
			rot.setX( dat[ Pitch ] );
		}

		if ( changed[ Roll ] )
		{
			dat[ Roll ] = clampEuler( dat[ Roll ] );
			rot.setZ( dat[ Roll ] );
		}

		if ( changed[ Yaw ] )
		{
			dat[ Yaw ] = clampEuler( dat[ Yaw ] );
			rot.setY( dat[ Yaw ] );
		}
	}
} // namespace fgl::engine::gui
