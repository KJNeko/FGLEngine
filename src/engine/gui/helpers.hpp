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
		float dat[ 3 ] { rot.pitch(), rot.roll(), rot.yaw() };
		const float c_dat[ 3 ] { dat[ 0 ], dat[ 1 ], dat[ 2 ] };

		constexpr float speed { 0.01f };

		ImGui::DragFloat3( label, dat, speed );

		const float diff[ 3 ] { c_dat[ 0 ] - dat[ 0 ], c_dat[ 1 ] - dat[ 1 ], c_dat[ 2 ] - dat[ 2 ] };
		constexpr float epsilon { std::numeric_limits< float >::epsilon() };
		const bool changed[ 3 ] { diff[ 0 ] > epsilon || diff[ 0 ]< epsilon, diff[ 1 ] > epsilon
			                      || diff[ 1 ]< epsilon, diff[ 2 ] > epsilon || diff[ 2 ] < epsilon };

		if ( changed[ 0 ] )
		{
			rot.pitch() += diff[ 0 ];
		}

		if ( changed[ 1 ] )
		{
			rot.roll() += diff[ 1 ];
		}

		if ( changed[ 2 ] )
		{
			rot.yaw() += diff[ 2 ];
		}
	}

} // namespace fgl::engine::gui
