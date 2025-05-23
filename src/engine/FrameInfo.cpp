//
// Created by kj16609 on 7/20/24.
//

#include "FrameInfo.hpp"

#include "camera/Camera.hpp"
#include "camera/GBufferSwapchain.hpp"

namespace fgl::engine
{

	descriptors::DescriptorSet& FrameInfo::getGBufferDescriptor() const
	{
		return camera->getSwapchain().getGBufferDescriptor( in_flight_idx );
	}

	descriptors::DescriptorSet& FrameInfo::getCameraDescriptor() const
	{
		return camera->getDescriptor( in_flight_idx );
	}

	void FrameInfo::bindCamera( [[maybe_unused]] Pipeline& pipeline )
	{
		//TODO: This
	}

} // namespace fgl::engine
