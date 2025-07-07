//
// Created by kj16609 on 8/8/24.
//

#pragma once

#include "interface/GameObjectComponent.hpp"

namespace fgl::engine
{

	using LightMask = uint32_t;

	COMPONENT_CLASS( PointLightComponent, PointLightComponentID )
	{
		glm::vec3 m_color;
		float m_brightness;

		//! Objects that fit this mask should be lit by this light
		LightMask m_lighting_mask;

		//! Objects that fit this mask should not block this light.
		LightMask m_block_mask;

		//! Objects that fit this mask should not be shadowed by this light
		LightMask m_shadowed_mask;

	  public:
	};

} // namespace fgl::engine
