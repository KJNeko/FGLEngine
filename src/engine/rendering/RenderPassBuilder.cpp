//
// Created by kj16609 on 12/31/23.
//

#include "Device.hpp"
#include "RenderPassBuilder.hpp"

namespace fgl::engine
{

	vk::raii::RenderPass RenderPassBuilder::create()
	{
		auto& device { Device::getInstance() };

		vk::RenderPassCreateInfo info;

		info.attachmentCount = static_cast< std::uint32_t >( attachment_descriptions.size() );
		info.pAttachments = attachment_descriptions.data();
		info.subpassCount = static_cast< std::uint32_t >( subpass_descriptions.size() );
		info.pSubpasses = subpass_descriptions.data();
		info.dependencyCount = static_cast< std::uint32_t >( dependencies.size() );
		info.pDependencies = dependencies.data();

		return device->createRenderPass( info );
	}

} // namespace fgl::engine
