//
// Created by kj16609 on 7/23/24.
//

#include "CameraManager.hpp"

#include "Camera.hpp"
#include "engine/debug/DEBUG_NAMES.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"

namespace fgl::engine
{

	using namespace fgl::literals::size_literals;

	inline static std::unique_ptr< CameraManager > camera_manager_instance;

	CameraManager& CameraManager::getInstance()
	{
		if ( !camera_manager_instance )
		{
			camera_manager_instance = std::unique_ptr< CameraManager >( new CameraManager() );
		}

		return *camera_manager_instance;
	}

	std::vector< std::weak_ptr< Camera > >& CameraManager::getCameras()
	{
		return cameras;
	}

	std::shared_ptr< Camera >& CameraManager::getPrimary()
	{
		return m_primary_camera;
	}

	CameraManager::CameraManager() :
	  m_data_buffer( 4_KiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible )
	{
		Camera::initCameraRenderer();
		debug::setDebugDrawingCamera( *getPrimary() );

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