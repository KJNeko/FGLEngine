//
// Created by kj16609 on 7/23/24.
//

#include "CameraManager.hpp"

#include "Camera.hpp"
#include "GBufferRenderer.hpp"
#include "engine/debug/DEBUG_NAMES.hpp"
#include "engine/math/literals/size.hpp"

namespace fgl::engine
{

	using namespace fgl::literals::size_literals;

	std::vector< std::weak_ptr< Camera > >& CameraManager::getCameras()
	{
		return cameras;
	}

	std::shared_ptr< Camera >& CameraManager::getPrimary()
	{
		return m_primary_camera;
	}

	CameraManager::CameraManager() :
	  m_renderer( std::make_shared< GBufferRenderer >() ),
	  m_data_buffer( 4_KiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible )
	{
		m_primary_camera = createCamera( { 1920, 1080 } );
		m_primary_camera->setName( CAMERA_EDITOR_NAME );
	}

	std::shared_ptr< Camera > CameraManager::createCamera( const vk::Extent2D extent )
	{
		std::shared_ptr< Camera > camera { new Camera( extent, m_data_buffer, m_renderer ) };

		this->cameras.emplace_back( camera );

		return camera;
	}

	CameraManager::~CameraManager()
	{
		m_primary_camera.reset();
	}
} // namespace fgl::engine