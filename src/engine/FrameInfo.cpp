//
// Created by kj16609 on 7/20/24.
//

#include "FrameInfo.hpp"

#include "camera/Camera.hpp"
#include "camera/CameraSwapchain.hpp"

namespace fgl::engine
{

	descriptors::DescriptorSet& FrameInfo::getGBufferDescriptor()
	{
		return camera_data.camera->getSwapchain().getGBufferDescriptor( frame_idx );
	}

	descriptors::DescriptorSet& FrameInfo::getCameraDescriptor()
	{
		return camera_data.camera->getDescriptor( frame_idx );
	}

} // namespace fgl::engine
