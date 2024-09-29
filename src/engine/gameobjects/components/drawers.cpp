//
// Created by kj16609 on 8/13/24.
//

#include "drawers.hpp"

#include <imgui.h>

namespace fgl::engine
{

	void InputRotation( const char* label, Rotation& rot, float speed )
	{
		enum Axis
		{
			Pitch = 0,
			Yaw = 1,
			Roll = 2
		};

		glm::vec3 dat { glm::degrees( rot.euler() ) };
		const glm::vec3 c_dat { dat };

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
			rot.setX( dat[ Pitch ] );
		}

		if ( changed[ Roll ] )
		{
			rot.setZ( dat[ Roll ] );
		}

		if ( changed[ Yaw ] )
		{
			rot.setY( dat[ Yaw ] );
		}
	}

	void drawComponentTransform( ComponentTransform& transform )
	{
		if ( ImGui::CollapsingHeader( "Transform" ) )
		{
			constexpr float speed { 1.0f };

			ImGui::DragFloat3( "Position", &transform.translation.x, speed );

			InputRotation( "Rotation", transform.rotation, speed );

			ImGui::DragFloat3( "Scale", &transform.scale.x, speed );
		}
	}

} // namespace fgl::engine
