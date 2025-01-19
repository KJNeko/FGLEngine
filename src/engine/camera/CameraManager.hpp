//
// Created by kj16609 on 7/23/24.
//

#pragma once
#include <vector>

#include "Camera.hpp"
#include "engine/memory/buffers/Buffer.hpp"

namespace fgl::engine
{
	namespace descriptors
	{
		class DescriptorSet;
	}

	class CameraManager
	{
		std::shared_ptr< GBufferRenderer > m_renderer;
		memory::Buffer m_data_buffer;

		std::shared_ptr< Camera > m_primary_camera { nullptr };

		std::vector< std::weak_ptr< Camera > > cameras {};

	  public:

		CameraManager();
		~CameraManager();

		std::vector< std::weak_ptr< Camera > >& getCameras();

		std::shared_ptr< Camera >& getPrimary();

		std::shared_ptr< Camera > createCamera( vk::Extent2D extent );
	};

} // namespace fgl::engine