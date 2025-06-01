//
// Created by kj16609 on 1/30/25.
//

#include "helpers.hpp"

#include "engine/primitives/rotation/QuatRotation.hpp"
#include "primitives/rotation/EulerRotation.hpp"

namespace fgl::engine::gui
{

	float clampEuler( const float value )
	{
		constexpr float max { 180.0f };
		constexpr float wrap { 360.0f };

// ignoring -Wanalyzer-too-complex here because of recursion
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wanalyzer-too-complex"
		if ( value >= max ) return clampEuler( value - wrap );
		if ( value <= -max ) return clampEuler( value + wrap );
#pragma GCC diagnostic pop

		return value;
	}

	void dragFloat3Rot( const char* const label, EulerRotation& rot )
	{
		enum Axis
		{
			Pitch = 0,
			Yaw = 1,
			Roll = 2
		};

		const glm::vec3 c_dat { rot.vec() };
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
			rot.x = dat[ Pitch ];
		}

		if ( changed[ Yaw ] )
		{
			dat[ Yaw ] = clampEuler( dat[ Yaw ] );
			rot.y = dat[ Yaw ];
		}

		if ( changed[ Roll ] )
		{
			dat[ Roll ] = clampEuler( dat[ Roll ] );
			rot.z = dat[ Roll ];
		}
	}
} // namespace fgl::engine::gui
