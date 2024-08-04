//
// Created by kj16609 on 6/5/24.
//

#pragma once

#include "safe_include.hpp"

namespace fgl::engine::gui
{
	inline void dragFloat3( const char* const label, glm::vec3& vec )
	{
		ImGui::DragFloat3( label, &vec.x );
	}

	inline void dragFloat3Rot( const char* const label, Rotation& rot )
	{
		enum Axis
		{
			Pitch = 0,
			Yaw = 1,
			Roll = 2
		};

		glm::vec3 dat { rot.euler() };
		const glm::vec3 c_dat { dat };

		constexpr float speed { 0.01f };

		assert( &dat.x + 1 == &dat.y );
		assert( &dat.y + 1 == &dat.z );

		ImGui::DragFloat3( label, &dat.x, speed );

		const glm::vec3 diff { c_dat - dat };
		constexpr float epsilon { std::numeric_limits< float >::epsilon() };

		const glm::vec< 3, bool > changed_high { glm::greaterThanEqual( diff, glm::vec3( epsilon ) ) };
		const glm::vec< 3, bool > changed_low { glm::lessThanEqual( diff, glm::vec3( -epsilon ) ) };
		const glm::vec< 3, bool > changed { changed_high || changed_low };

		if ( changed[ Pitch ] )
		{
			rot.pitch() = dat[ Pitch ];
		}

		if ( changed[ Roll ] )
		{
			rot.roll() = dat[ Roll ];
		}

		if ( changed[ Yaw ] )
		{
			rot.yaw() = dat[ Yaw ];
		}
	}

} // namespace fgl::engine::gui
