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
			Roll = 1,
			Yaw = 2
		};

		float dat[ 3 ] { rot.pitch(), rot.roll(), rot.yaw() };
		const float c_dat[ 3 ] { dat[ Pitch ], dat[ Roll ], dat[ Yaw ] };

		constexpr float speed { 0.01f };

		ImGui::DragFloat3( label, dat, speed );

		const float diff[ 3 ] { c_dat[ Pitch ] - dat[ Pitch ], c_dat[ Roll ] - dat[ Roll ], c_dat[ Yaw ] - dat[ Yaw ] };
		constexpr float epsilon { std::numeric_limits< float >::epsilon() };
		const bool changed[ 3 ] { diff[ Pitch ] > epsilon || diff[ Pitch ]< -epsilon, diff[ Roll ] > epsilon
			                      || diff[ Roll ]< -epsilon, diff[ Yaw ] > epsilon || diff[ Yaw ] < -epsilon };

		if ( changed[ Pitch ] )
		{
			rot.pitch() += diff[ Pitch ];
		}

		if ( changed[ Roll ] )
		{
			rot.roll() += diff[ Roll ];
		}

		if ( changed[ Yaw ] )
		{
			rot.yaw() += diff[ Yaw ];
		}
	}

} // namespace fgl::engine::gui
