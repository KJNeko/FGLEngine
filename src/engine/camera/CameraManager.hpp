//
// Created by kj16609 on 7/23/24.
//

#pragma once
#include <vector>

#include "Camera.hpp"
#include "engine/buffers/UniqueFrameSuballocation.hpp"

namespace fgl::engine
{
	namespace descriptors
	{
		class DescriptorSet;
	}

	class CameraManager
	{
		memory::Buffer m_data_buffer;

		std::shared_ptr< Camera > m_primary_camera { nullptr };

		std::vector< std::weak_ptr< Camera > > cameras {};

	  public:

		std::vector< std::weak_ptr< Camera > >& getCameras();

		Camera& getPrimary();

		CameraManager();

		std::shared_ptr< Camera > createCamera( vk::Extent2D extent );
	};

} // namespace fgl::engine