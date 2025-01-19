//
// Created by kj16609 on 11/27/23.
//

#include "EngineContext.hpp"

#include <tracy/TracyC.h>
#include <tracy/TracyVulkan.hpp>

#include <chrono>
#include <iostream>

#include "KeyboardMovementController.hpp"
#include "assets/material/Material.hpp"
#include "camera/Camera.hpp"
#include "camera/CameraManager.hpp"
#include "camera/GBufferRenderer.hpp"
#include "debug/timing/FlameGraph.hpp"
#include "engine/assets/model/builders/SceneBuilder.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/flags.hpp"
#include "engine/math/Average.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"

namespace fgl::engine
{
	constexpr float MAX_DELTA_TIME { 0.5 };

	EngineContext::EngineContext() :
	  m_ubo_buffer_pool( 1_MiB, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_material_data_pool(
		  1_MiB,
		  vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
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

		memory::TransferManager::createInstance( device, 128_MiB );


		m_matrix_info_pool.setDebugName( "Matrix info pool" );
		m_draw_parameter_pool.setDebugName( "Draw parameter pool" );

		m_vertex_buffer->setDebugName( "Vertex buffer" );
		m_index_buffer->setDebugName( "Index buffer" );

		m_material_data_pool.setDebugName( "Material data pool" );

		initMaterialDataVec( m_material_data_pool );

		constexpr auto offset { 8.0f };
		constexpr std::size_t grid_size { 6 };
		constexpr float factor_offset { 1.0f / static_cast< float >( grid_size ) };

		for ( std::size_t x = 0; x < grid_size; ++x )
			for ( std::size_t y = 0; y < grid_size; ++y )
			{
				const std::filesystem::path path {
					"/home/kj16609/Desktop/Projects/cxx/Mecha/src/assets/PBRSphere.gltf"
				};

				SceneBuilder builder { *m_vertex_buffer, *m_index_buffer };

				builder.loadScene( path );

				std::vector< GameObject > objs { builder.getGameObjects() };

				for ( auto& obj : objs )
				{
					auto model_components { obj.getComponents< ModelComponent >() };

					for ( const auto& model_component : model_components )
					{
						auto& prims = ( *model_component )->m_primitives;

						for ( auto& prim : prims )
						{
							auto& pbr { prim.m_material->properties.pbr };
							pbr.roughness_factor = x * factor_offset;
							pbr.metallic_factor = y * factor_offset;
							prim.m_material->update();
						}
					}

					obj.getTransform().translation = WorldCoordinate( 10.0 + x * offset, 10.0 + y * offset, 0.0f );

					m_game_objects_root.addGameObject( std::move( obj ) );
				}
			}
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
		const std::chrono::duration< double, std::chrono::seconds::period > time_diff { now - last_tick };
		last_tick = now;

		// Convert from ms to s
		m_delta_time = time_diff.count();
	}

	void EngineContext::tickSimulation()
	{
		ZoneScoped;
		auto timer = debug::timing::push( "Tick Simulation" );
		// TODO: This is where we'll start doing physics stuff.
		// The first step here should be culling things that aren't needed to be ticked.
		// Perhaps implementing a tick system that doesn't care about the refresh rate might be good?
		// That way we can still tick consistantly without actually needing to render anything.
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
		if ( auto& command_buffer = m_renderer.beginFrame(); *command_buffer )
		{
			const auto timer = debug::timing::push( "Render Frame" );
			const FrameIndex frame_index { m_renderer.getFrameIndex() };
			const PresentIndex present_idx { m_renderer.getPresentIndex() };

			FrameInfo frame_info { frame_index,
				                   present_idx,
				                   m_delta_time,
				                   command_buffer,
				                   nullptr, // Camera
				                   m_camera_manager.getCameras(),
				                   // global_descriptor_sets[ frame_index ],
				                   m_game_objects_root,
				                   m_renderer.getCurrentTracyCTX(),
				                   m_matrix_info_pool,
				                   m_draw_parameter_pool,
				                   *this->m_vertex_buffer,
				                   *this->m_index_buffer,
				                   // m_renderer.getSwapChain().getInputDescriptor( present_idx ),
				                   this->m_renderer.getSwapChain() };

			{
				ZoneScopedN( "Pre frame hooks" );
				for ( const auto& hook : pre_frame_hooks ) hook( frame_info );
			}

			TracyVkCollect( frame_info.tracy_ctx, *command_buffer );

			//TODO: Setup semaphores to make this pass not always required.
			memory::TransferManager::getInstance().recordOwnershipTransferDst( command_buffer );

			for ( const auto& hook : early_render_hooks ) hook( frame_info );
			//TODO: Add some way of 'activating' cameras. We don't need to render cameras that aren't active.
			renderCameras( frame_info );
			for ( const auto& hook : render_hooks ) hook( frame_info );

			m_renderer.beginSwapchainRendererPass( command_buffer );

			m_gui_system.pass( frame_info );

			for ( const auto& hook : late_render_hooks ) hook( frame_info );

			m_renderer.endSwapchainRendererPass( command_buffer );

			m_renderer.endFrame();

			memory::TransferManager::getInstance().dump();

			{
				ZoneScopedN( "Post frame hooks" );
				for ( const auto& hook : post_frame_hooks ) hook( frame_info );
			}

			flags::resetFlags();

			FrameMark;
		}

		//Trash handling
		descriptors::deleteQueuedDescriptors();
	}

	void EngineContext::finishFrame()
	{}

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

	EngineContext::~EngineContext()
	{
		// Destroy all objects
		m_game_objects_root.clear();
		destroyMaterialDataVec();

		for ( const auto& hook : destruction_hooks ) hook();
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
