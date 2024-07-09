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
#include "assets/stores.hpp"
#include "buffers/HostSingleT.hpp"
#include "engine/Average.hpp"
#include "engine/assets/TransferManager.hpp"
#include "engine/buffers/UniqueFrameSuballocation.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"
#include "engine/model/prebuilt/terrainModel.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/systems/EntityRendererSystem.hpp"
#include "gui/core.hpp"
#include "model/builders/SceneBuilder.hpp"

namespace fgl::engine
{
	constexpr float MAX_DELTA_TIME { 0.5 };

	EngineContext::EngineContext()
	{
		ZoneScoped;
		using namespace fgl::literals::size_literals;

		memory::TransferManager::createInstance( device, 512_MiB );

#if ENABLE_IMGUI
		initImGui();
#endif
		loadGameObjects();
	}

	static Average< float, 60 * 15 > rolling_ms_average;

	void EngineContext::run()
	{
		TracyCZoneN( TRACY_PrepareEngine, "Inital Run", true );
		std::cout << "Starting main loop run" << std::endl;
		using namespace fgl::literals::size_literals;
		memory::Buffer global_ubo_buffer { 512_KiB,
			                               vk::BufferUsageFlagBits::eUniformBuffer,
			                               vk::MemoryPropertyFlagBits::eHostVisible }; // 512 KB

		PerFrameSuballocation< HostSingleT< CameraInfo > > camera_info { global_ubo_buffer,
			                                                             SwapChain::MAX_FRAMES_IN_FLIGHT };

		PerFrameSuballocation< HostSingleT< PointLight > > point_lights { global_ubo_buffer,
			                                                              SwapChain::MAX_FRAMES_IN_FLIGHT };

		std::shared_ptr< Texture > debug_tex {
			getTextureStore().load( "assets/textures/DebugTexture.png", vk::Format::eR8G8B8A8Unorm )
		};

		constexpr std::uint32_t matrix_default_size { 64_MiB };
		constexpr std::uint32_t draw_parameter_default_size { 64_MiB };

		std::vector< memory::Buffer > matrix_info_buffers {};

		std::vector< memory::Buffer > draw_parameter_buffers {};

		std::vector< descriptors::DescriptorSet > global_descriptor_sets {};

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			matrix_info_buffers.emplace_back(
				matrix_default_size,
				vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible );

			draw_parameter_buffers.emplace_back(
				draw_parameter_default_size,
				vk::BufferUsageFlagBits::eIndirectBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible );

			global_descriptor_sets.emplace_back( GlobalDescriptorSet::createLayout() );
		}

		for ( std::uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			global_descriptor_sets[ i ].setMaxIDX( 2 );
			global_descriptor_sets[ i ].bindUniformBuffer( 0, camera_info[ i ] );
			global_descriptor_sets[ i ].bindUniformBuffer( 2, point_lights[ i ] );
			global_descriptor_sets[ i ].update();
		}

		Camera camera {};
		debug::setDebugDrawingCamera( camera );

		auto viewer { GameObject::createGameObject() };

		viewer.getTransform().translation = WorldCoordinate( constants::WORLD_CENTER + glm::vec3( 0.0f, 0.0f, 2.5f ) );

		KeyboardMovementController camera_controller {};

		auto current_time { std::chrono::high_resolution_clock::now() };

		auto previous_frame_start { std::chrono::high_resolution_clock::now() };

		//camera.setOrthographicProjection( -aspect, aspect, -1, 1, -1, 1 );
		const float aspect { m_renderer.getAspectRatio() };
		camera.setPerspectiveProjection( glm::radians( 90.0f ), aspect, constants::NEAR_PLANE, constants::FAR_PLANE );

		const auto old_aspect_ratio { m_renderer.getAspectRatio() };

		TracyCZoneEnd( TRACY_PrepareEngine );

		while ( !m_window.shouldClose() )
		{
			memory::TransferManager::getInstance().submitNow();

			ZoneScopedN( "Poll" );
			glfwPollEvents();

			const auto new_time { std::chrono::high_resolution_clock::now() };

			{
				//Calculate time change from previous frame and add to accumulator
				const auto time_diff { new_time - previous_frame_start };
				rolling_ms_average.push(
					static_cast< float >( std::chrono::duration_cast< std::chrono::microseconds >( time_diff ).count() )
					/ 1000.0f );
				previous_frame_start = new_time;
			}

			auto delta_time { std::chrono::duration< float >( new_time - current_time ).count() };

			current_time = new_time;
			delta_time = glm::min( delta_time, MAX_DELTA_TIME );

			if ( old_aspect_ratio != m_renderer.getAspectRatio() )
			{
				camera.setPerspectiveProjection(
					glm::radians( 90.0f ), m_renderer.getAspectRatio(), constants::NEAR_PLANE, constants::FAR_PLANE );
			}

			camera_controller.moveInPlaneXZ( m_window.window(), delta_time, viewer );
			camera.setView( viewer.getPosition(), viewer.getRotation() );

			if ( auto [ command_buffer, gui_command_buffer ] = m_renderer.beginFrame(); *command_buffer )
			{
				ZoneScopedN( "Render" );
				//Update
				const std::uint16_t frame_index { m_renderer.getFrameIndex() };

				const auto view_frustum { camera.getFrustumBounds() };

				FrameInfo frame_info { frame_index,
					                   delta_time,
					                   command_buffer,
					                   gui_command_buffer,
					                   { camera, viewer.getTransform() },
					                   global_descriptor_sets[ frame_index ],
					                   m_game_objects_root,
					                   m_renderer.getCurrentTracyCTX(),
					                   matrix_info_buffers[ frame_index ],
					                   draw_parameter_buffers[ frame_index ],
					                   *this->m_vertex_buffer,
					                   *this->m_index_buffer,
					                   m_renderer.getGBufferDescriptor( frame_index ),
					                   m_renderer.getGBufferCompositeDescriptor( frame_index ),
					                   view_frustum,
					                   this->m_renderer.getSwapChain() };

#if TRACY_ENABLE
				//auto& tracy_ctx { frame_info.tracy_ctx };
#endif

				CameraInfo current_camera_info { .projection = camera.getProjectionMatrix(),
					                             .view = camera.getViewMatrix(),
					                             .inverse_view = camera.getInverseViewMatrix() };

				camera_info[ frame_index ] = current_camera_info;

				m_culling_system.startPass( frame_info );
				TracyVkCollect( frame_info.tracy_ctx, *command_buffer );

				memory::TransferManager::getInstance().recordOwnershipTransferDst( command_buffer );

				m_culling_system.wait();

				m_renderer.beginSwapchainRendererPass( command_buffer );

				m_terrain_system.pass( frame_info );

				m_entity_renderer.pass( frame_info );

				m_composition_system.pass( frame_info );

				m_gui_system.pass( frame_info );

				m_renderer.endSwapchainRendererPass( command_buffer );

				m_renderer.endFrame();

				memory::TransferManager::getInstance().dump();

				FrameMark;
			}

			using namespace std::chrono_literals;
			std::this_thread::sleep_for( 13ms );
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

	void EngineContext::initImGui()
	{
#if ENABLE_IMGUI
		gui::initGui( m_window, m_renderer );
#endif
	}

	EngineContext::~EngineContext()
	{
#if ENABLE_IMGUI
		gui::cleanupImGui();
#endif
	}

} // namespace fgl::engine
