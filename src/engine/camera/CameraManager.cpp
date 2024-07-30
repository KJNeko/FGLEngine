//
// Created by kj16609 on 7/23/24.
//

#include "CameraManager.hpp"

#include "Camera.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/gui/gui_window_names.hpp"
#include "engine/literals/size.hpp"

namespace fgl::engine
{

	using namespace fgl::literals::size_literals;

	std::vector< std::weak_ptr< Camera > >& CameraManager::getCameras()
	{
		return cameras;
	}

	Camera& CameraManager::getPrimary()
	{
		return *m_primary_camera;
	}

	CameraManager::CameraManager() :
	  m_data_buffer( 4_KiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible )
	{
		Camera::initCameraRenderer();
		debug::setDebugDrawingCamera( getPrimary() );

		m_primary_camera = createCamera( { 1920, 1080 } );
		m_primary_camera->setName( CAMERA_EDITOR_NAME );
	}

	std::shared_ptr< Camera > CameraManager::createCamera( const vk::Extent2D extent )
	{
		std::shared_ptr< Camera > camera { new Camera( extent, m_data_buffer ) };

		this->cameras.emplace_back( camera );

		return camera;
	}
} // namespace fgl::engine