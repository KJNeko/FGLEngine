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

		glm::vec3 dat { glm::degrees( rot.euler() ) };
		const glm::vec3 c_dat { dat };

		constexpr float speed { 1.0f };

		assert( &dat.x + 1 == &dat.y );
		assert( &dat.y + 1 == &dat.z );

		ImGui::DragFloat3( label, &dat.x, speed );

		const glm::vec3 diff { c_dat - dat };
		constexpr float epsilon { std::numeric_limits< float >::epsilon() };

		const glm::vec< 3, bool > changed_high { glm::greaterThanEqual( diff, glm::vec3( epsilon ) ) };
		const glm::vec< 3, bool > changed_low { glm::lessThanEqual( diff, glm::vec3( -epsilon ) ) };
		const glm::vec< 3, bool > changed { changed_high || changed_low };

		// Convert back to radians
		dat = glm::radians( dat );

		if ( changed[ Pitch ] )
		{
			//TODO: rot.xAngle() = dat[ Pitch ];
		}

		if ( changed[ Roll ] )
		{
			//TODO: rot.zAngle() = dat[ Roll ];
		}

		if ( changed[ Yaw ] )
		{
			//TODO: rot.yAngle() = dat[ Yaw ];
		}
	}

} // namespace fgl::engine::gui
