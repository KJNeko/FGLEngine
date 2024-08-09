//
// Created by kj16609 on 8/2/24.
//

#pragma once
#include <memory>

#include "engine/gameobjects/components/interface/GameObjectComponent.hpp"

namespace fgl::engine
{
	class Camera;

	COMPONENT_CLASS( CameraComponent, CameraComponentID )
	{
		std::shared_ptr< Camera > m_camera;

	  public:

		CameraComponent() = delete;
		CameraComponent( std::shared_ptr< Camera > & camera );
		~CameraComponent();
	};

	static_assert( is_component< CameraComponent > );

} // namespace fgl::engine