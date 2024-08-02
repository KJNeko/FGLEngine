//
// Created by kj16609 on 8/2/24.
//

#pragma once
#include <memory>

#include "GameObjectComponent.hpp"

namespace fgl::engine
{
	class Camera;

	class CameraComponent final : public GameObjectComponent< 2 >
	{
		std::shared_ptr< Camera > m_camera;

	  public:

		CameraComponent() = delete;
		CameraComponent( std::shared_ptr< Camera >& camera );
		~CameraComponent();
	};

	static_assert( is_component< CameraComponent > );

} // namespace fgl::engine