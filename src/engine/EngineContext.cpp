//
// Created by kj16609 on 11/27/23.
//

#include "EngineContext.hpp"

#include <tracy/TracyC.h>
#include <tracy/TracyVulkan.hpp>

#include <chrono>
#include <iostream>

#include "KeyboardMovementController.hpp"
#include "camera/Camera.hpp"
#include "camera/CameraManager.hpp"
#include "camera/GBufferRenderer.hpp"
#include "debug/timing/FlameGraph.hpp"
#include "engine/assets/model/builders/SceneBuilder.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/flags.hpp"
#include "engine/math/Average.hpp"
#include "engine/math/literals/size.hpp"

namespace fgl::engine
{
	constexpr float MAX_DELTA_TIME { 0.5 };
	inline static EngineContext* instance { nullptr };

	EngineContext::EngineContext() :
	  m_ubo_buffer_pool( 1_MiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_matrix_info_pool(
		  256_MiB,
		  vk::BufferUsageFlagBits::eVertexBuffer,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_draw_parameter_pool(
		  128_MiB,
		  vk::BufferUsageFlagBits::eIndirectBuffer,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_delta_time( 0.0 )
	{
		ZoneScoped;
		using namespace fgl::literals::size_literals;

		instance = this;

		// memory::TransferManager::createInstance( device, 128_MiB );

		m_matrix_info_pool.setDebugName( "Matrix info pool" );
		m_draw_parameter_pool.setDebugName( "Draw parameter pool" );
	}

	static Average< float, 60 * 15 > rolling_ms_average;

	void EngineContext::processInput()
	{
		auto timer = debug::timing::push( "Process Inputs" );
		glfwPollEvents();
	}

	void EngineContext::tickDeltaTime()
	{
		ZoneScoped;
		// Get delta time
		const auto now { fgl::Clock::now() };
		const std::chrono::duration< DeltaTime, std::chrono::seconds::period > time_diff { now - m_last_tick };
		m_last_tick = now;

		// Convert from ms to s
		m_delta_time = time_diff.count();
	}

	World EngineContext::tickSimulation()
	{
		ZoneScoped;
		auto timer = debug::timing::push( "Tick Simulation" );
		// TODO: This is where we'll start doing physics stuff.
		// The first step here should be culling things that aren't needed to be ticked.
		// Perhaps implementing a tick system that doesn't care about the refresh rate might be good?
		// That way we can still tick consistantly without actually needing to render anything.

		return {};
	}

	void EngineContext::renderCameras( FrameInfo frame_info )
	{
		ZoneScoped;
		auto timer = debug::timing::push( "Render Cameras" );
		for ( auto& current_camera_ptr : m_camera_manager.getCameras() )
		{
			if ( current_camera_ptr.expired() ) continue;

			auto sh_camera { current_camera_ptr.lock() };

			Camera& current_camera { *sh_camera };

			current_camera.pass( frame_info );
		}
	}

	void EngineContext::renderFrame()
	{
		ZoneScoped;
		if ( auto command_buffers_o = m_renderer.beginFrame(); command_buffers_o.has_value() )
		{
			const auto timer = debug::timing::push( "Render Frame" );
			const FrameIndex frame_index { m_renderer.getFrameIndex() };
			const PresentIndex present_idx { m_renderer.getPresentIndex() };

			auto& command_buffers { command_buffers_o.value() };

			FrameInfo frame_info { frame_index,
				                   present_idx,
				                   m_delta_time,
				                   command_buffers,
				                   nullptr, // Camera
				                   m_camera_manager.getCameras(),
				                   // global_descriptor_sets[ frame_index ],
				                   m_model_manager,
				                   m_renderer.getCurrentTracyCTX(),
				                   m_matrix_info_pool,
				                   m_draw_parameter_pool,
				                   // m_renderer.getSwapChain().getInputDescriptor( present_idx ),
				                   this->m_renderer.getSwapChain() };

			{
				ZoneScopedN( "Pre frame hooks" );
				for ( const auto& hook : m_pre_frame_hooks ) hook( frame_info );
			}

			TracyVkCollect( frame_info.tracy_ctx, **command_buffers.transfer_cb );

			//TODO: Setup semaphores to make this pass not always required.
			m_transfer_manager.recordOwnershipTransferDst( command_buffers.transfer_cb );

			for ( const auto& hook : m_early_render_hooks ) hook( frame_info );
			//TODO: Add some way of 'activating' cameras. We don't need to render cameras that aren't active.
			renderCameras( frame_info );
			for ( const auto& hook : m_render_hooks ) hook( frame_info );

			m_renderer.beginSwapchainRendererPass( command_buffers.imgui_cb );

			m_gui_system.pass( frame_info );

			for ( const auto& hook : m_late_render_hooks ) hook( frame_info );

			m_renderer.endSwapchainRendererPass( command_buffers.imgui_cb );

			m_renderer.endFrame( command_buffers );

			m_transfer_manager.dump();
			m_device.getCmdBufferPool().advanceInFlight();

			{
				ZoneScopedN( "Post frame hooks" );
				for ( const auto& hook : m_post_frame_hooks ) hook( frame_info );
			}

			flags::resetFlags();

			FrameMark;
		}

		//Trash handling
		descriptors::deleteQueuedDescriptors();
	}

	void EngineContext::finishFrame()
	{}

	void EngineContext::waitIdle()
	{
		m_device->waitIdle();
	}

	Window& EngineContext::getWindow()
	{
		return m_window;
	}

	float EngineContext::getWindowAspectRatio()
	{
		return m_renderer.getAspectRatio();
	}

	CameraManager& EngineContext::cameraManager()
	{
		return m_camera_manager;
	}

	MaterialManager& EngineContext::getMaterialManager()
	{
		return m_material_manager;
	}

	EngineContext::~EngineContext()
	{
		log::info( "Destroying EngineContext" );

		// Destroy all objects
		m_game_objects_root.clear();

		descriptors::deleteQueuedDescriptors();

		log::info( "Performing {} destruction hooks", m_destruction_hooks.size() );

		for ( const auto& hook : m_destruction_hooks ) hook();
	}

	EngineContext& EngineContext::getInstance()
	{
		assert( instance );
		return *instance;
	}

	bool EngineContext::good()
	{
		return !m_window.shouldClose();
	}

	void EngineContext::handleTransfers()
	{
		memory::TransferManager::getInstance().submitNow();
	}

} // namespace fgl::engine
