//
// Created by kj16609 on 12/31/23.
//

#include "RenderPass.hpp"

#include "Device.hpp"

namespace fgl::engine
{

	vk::RenderPass RenderPass::create()
	{
		auto device { Device::getInstance().device() };

		vk::RenderPassCreateInfo info;

		info.attachmentCount = static_cast< std::uint32_t >( attachment_descriptions.size() );
		info.pAttachments = attachment_descriptions.data();
		info.subpassCount = static_cast< std::uint32_t >( subpass_descriptions.size() );
		info.pSubpasses = subpass_descriptions.data();
		info.dependencyCount = static_cast< std::uint32_t >( dependencies.size() );
		info.pDependencies = dependencies.data();

		vk::RenderPass render_pass {};

		if ( device.createRenderPass( &info, nullptr, &render_pass ) != vk::Result::eSuccess ) [[unlikely]]
		{
			throw std::runtime_error( "failed to create render pass!" );
		}
		else
			return render_pass;
	}

} // namespace fgl::engine
