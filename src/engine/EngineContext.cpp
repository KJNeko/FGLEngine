//
// Created by kj16609 on 11/27/23.
//

#include "EngineContext.hpp"

#include <glm/glm.hpp>
#include <tracy/TracyC.h>
#include <tracy/TracyVulkan.hpp>

#include <chrono>
#include <iostream>

#include "KeyboardMovementController.hpp"
#include "buffers/HostSingleT.hpp"
#include "camera/Camera.hpp"
#include "camera/CameraManager.hpp"
#include "engine/Average.hpp"
#include "engine/assets/TransferManager.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"
#include "model/builders/SceneBuilder.hpp"

namespace fgl::engine
{
	constexpr float MAX_DELTA_TIME { 0.5 };

	EngineContext::EngineContext() :
	  m_ubo_buffer_pool( 512_KiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_matrix_info_pool(
		  2_MiB,
		  vk::BufferUsageFlagBits::eVertexBuffer,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_draw_parameter_pool(
		  1_MiB,
		  vk::BufferUsageFlagBits::eIndirectBuffer,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_delta_time( 0.0 )
	{
		ZoneScoped;
		using namespace fgl::literals::size_literals;

		memory::TransferManager::createInstance( device, 128_MiB );
	}

	static Average< float, 60 * 15 > rolling_ms_average;

	void EngineContext::processInput()
	{
		glfwPollEvents();
	}

	void EngineContext::tickDeltaTime()
	{
		// Get delta time
		const auto now { std::chrono::high_resolution_clock::now() };
		const std::chrono::duration< double, std::chrono::seconds::period > time_diff { now - last_tick };
		last_tick = now;

		// Convert from ms to s
		m_delta_time = time_diff.count();
	}

	void EngineContext::tickSimulation()
	{
		// TODO: This is where we'll start doing physics stuff.
		// The first step here should be culling things that aren't needed to be ticked.
		// Perhaps implementing a tick system that doesn't care about the refresh rate might be good?
		// That way we can still tick consistantly without actually needing to render anything.
	}

	void EngineContext::renderCameras( FrameInfo frame_info )
	{
		auto& camera_manager { CameraManager::getInstance() };
		for ( auto& current_camera_ptr : camera_manager.getCameras() )
		{
			if ( current_camera_ptr.expired() ) continue;

			auto sh_camera { current_camera_ptr.lock() };

			Camera& current_camera { *sh_camera };

			current_camera.pass( frame_info );
		}
	}

	void EngineContext::renderFrame()
	{
		if ( auto& command_buffer = m_renderer.beginFrame(); *command_buffer )
		{
			ZoneScopedN( "Render" );
			const FrameIndex frame_index { m_renderer.getFrameIndex() };
			const PresentIndex present_idx { m_renderer.getPresentIndex() };

			auto& camera_manager { CameraManager::getInstance() };

			FrameInfo frame_info { frame_index,
				                   present_idx,
				                   m_delta_time,
				                   command_buffer,
				                   nullptr, // Camera
				                   camera_manager.getCameras(),
				                   // global_descriptor_sets[ frame_index ],
				                   m_game_objects_root,
				                   m_renderer.getCurrentTracyCTX(),
				                   m_matrix_info_pool,
				                   m_draw_parameter_pool,
				                   *this->m_vertex_buffer,
				                   *this->m_index_buffer,
				                   m_renderer.getSwapChain().getInputDescriptor( present_idx ),
				                   this->m_renderer.getSwapChain() };

			TracyVkCollect( frame_info.tracy_ctx, *command_buffer );

			//TODO: Setup semaphores to make this pass not always required.
			memory::TransferManager::getInstance().recordOwnershipTransferDst( command_buffer );

			//TODO: Add some way of 'activating' cameras. We don't need to render cameras that aren't active.
			renderCameras( frame_info );

			m_renderer.clearInputImage( command_buffer );

			//primary_camera
			//	.copyOutput( command_buffer, frame_index, m_renderer.getSwapChain().getInputImage( present_idx ) );

			m_renderer.beginSwapchainRendererPass( command_buffer );

			m_gui_system.pass( frame_info );

			// TODO: Implement some way we can record extra things into the command buffer during this stage.
			// We'll probably just use multiple command buffers and allow the caller to pass some in with flags on where to put them
			renderGui( frame_info );

			m_renderer.endSwapchainRendererPass( command_buffer );

			m_renderer.endFrame();

			memory::TransferManager::getInstance().dump();

			FrameMark;
		}
	}

	Window& EngineContext::getWindow()
	{
		return m_window;
	}

	float EngineContext::getWindowAspectRatio()
	{
		return m_renderer.getAspectRatio();
	}

	void EngineContext::run()
	{
		TracyCZoneN( TRACY_PrepareEngine, "Inital Run", true );
		std::cout << "Starting main loop run" << std::endl;

		auto viewer { GameObject::createGameObject() };

		viewer.getTransform().translation = WorldCoordinate( constants::WORLD_CENTER + glm::vec3( 0.0f, 0.0f, 2.5f ) );

		KeyboardMovementController camera_controller {};

		auto current_time { std::chrono::high_resolution_clock::now() };

		auto previous_frame_start { std::chrono::high_resolution_clock::now() };

		camera_controller.moveInPlaneXZ( m_window.window(), 0.0, viewer );

		TracyCZoneEnd( TRACY_PrepareEngine );

		while ( good() )
		{
			memory::TransferManager::getInstance().submitNow();

			ZoneScopedN( "Poll" );
			glfwPollEvents();

			const auto new_time { std::chrono::high_resolution_clock::now() };

			auto delta_time { std::chrono::duration< float >( new_time - current_time ).count() };

			current_time = new_time;
			delta_time = glm::min( delta_time, MAX_DELTA_TIME );

			camera_controller.moveInPlaneXZ( m_window.window(), delta_time, viewer );

			renderFrame();

			using namespace std::chrono_literals;
//			std::this_thread::sleep_for( 13ms );
		}

		Device::getInstance().device().waitIdle();
	}

	void EngineContext::loadGameObjects()
	{
		ZoneScoped;
		std::cout << "Loading game objects" << std::endl;
		auto command_buffer { Device::getInstance().beginSingleTimeCommands() };

		{
			ZoneScopedN( "Load phyiscs test" );
			SceneBuilder builder { *m_vertex_buffer, *m_index_buffer };
			builder.loadScene( "assets/PhysicsTest.glb" );

			auto objects { builder.getGameObjects() };

			for ( auto& object : objects )
			{
				object.addFlag( IS_VISIBLE | IS_ENTITY );

				m_game_objects_root.addGameObject( std::move( object ) );
			}
		}

		Device::getInstance().endSingleTimeCommands( command_buffer );
		log::info( "Finished loading game object" );
	}

	EngineContext::~EngineContext()
	{
		cleanupImGui();
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
