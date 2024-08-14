//
// Created by kj16609 on 7/20/24.
//

#include "FrameInfo.hpp"

#include "camera/Camera.hpp"
#include "camera/CameraSwapchain.hpp"
#include "rendering/pipelines/Pipeline.hpp"

namespace fgl::engine
{

	descriptors::DescriptorSet& FrameInfo::getGBufferDescriptor() const
	{
		return camera->getSwapchain().getGBufferDescriptor( frame_idx );
	}

	descriptors::DescriptorSet& FrameInfo::getCameraDescriptor() const
	{
		return camera->getDescriptor( frame_idx );
	}

	void FrameInfo::bindCamera( internal::Pipeline& pipeline )
	{
		//TODO: This
	}

} // namespace fgl::engine
