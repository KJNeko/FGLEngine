//
// Created by kj16609 on 8/8/24.
//

#pragma once
#define ASSIGN_COMPONENT_ID( name, id ) constexpr ComponentID name { id };

#include <cstdint>

namespace fgl::engine
{
	using ComponentID = std::uint32_t;

	ASSIGN_COMPONENT_ID( ModelComponentID, 1 );

	ASSIGN_COMPONENT_ID( CameraComponentID, 2 );

	ASSIGN_COMPONENT_ID( PointLightComponentID, 10 );
	ASSIGN_COMPONENT_ID( DirectionalLightComponentID, 11 );

} // namespace fgl::engine
