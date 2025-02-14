//
// Created by kj16609 on 2/14/25.
//
#pragma once

#include <vulkan/vulkan_raii.hpp>

#include "CommandBufferPool.hpp"

namespace fgl::engine
{
	struct CommandBuffers
	{
		CommandBuffer imgui_cb;
		CommandBuffer transfer_cb;
		CommandBuffer render_cb;
		CommandBuffer composition_cb;

		CommandBuffers() = delete;

		CommandBuffers( std::vector< CommandBuffer >&& buffers );
	};

} // namespace fgl::engine
