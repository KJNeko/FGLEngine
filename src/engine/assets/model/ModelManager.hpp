//
// Created by kj16609 on 3/12/25.
//
#pragma once
#include "memory/buffers/vector/DeviceVector.hpp"
#include "rendering/PresentSwapChain.hpp"

namespace fgl::engine
{
	class ModelManager
	{
		//! The managed draw command buffer.
		/**
		 * Contains all draw commands for the rendering process. Updated by the CPU whenever a new model is created
		 */
		DeviceVector< vk::DrawIndexedIndirectCommand > m_draw_commands;

		//! Draw buffers the GPU compute shader writes into
		PerFrameArray< DeviceVector< vk::DrawIndexedIndirectCommand > > m_gpu_draw_commands {};

		//! Contains extra information such as model instance indexes by the compute shader
		PerFrameArray<> m_gpu_draw_info {};
	};
} // namespace fgl::engine