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
#include "engine/Average.hpp"
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
		initGlobalStagingBuffer( 512_MiB );
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
		Buffer global_ubo_buffer { 512_KiB,
			                       vk::BufferUsageFlagBits::eUniformBuffer,
			                       vk::MemoryPropertyFlagBits::eHostVisible }; // 512 KB

		PerFrameSuballocation< HostSingleT< CameraInfo > > camera_info { global_ubo_buffer,
			                                                             SwapChain::MAX_FRAMES_IN_FLIGHT };

		PerFrameSuballocation< HostSingleT< PointLight > > point_lights { global_ubo_buffer,
			                                                              SwapChain::MAX_FRAMES_IN_FLIGHT };

		Texture debug_tex { Texture::loadFromFile( "models/textures/DebugTexture.png" ) };
		Sampler sampler { vk::Filter::eLinear,
			              vk::Filter::eLinear,
			              vk::SamplerMipmapMode::eLinear,
			              vk::SamplerAddressMode::eClampToEdge };
		debug_tex.getImageView().getSampler() = std::move( sampler );

		constexpr std::uint32_t matrix_default_size { 64_MiB };
		constexpr std::uint32_t draw_parameter_default_size { 64_MiB };

		std::vector< Buffer > matrix_info_buffers {};

		std::vector< Buffer > draw_parameter_buffers {};

		std::vector< DescriptorSet > global_descriptor_sets {};

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

		viewer.m_transform.translation = WorldCoordinate( constants::WORLD_CENTER + glm::vec3( 0.0f, 0.0f, 2.5f ) );

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

			if ( auto command_buffer = m_renderer.beginFrame(); command_buffer )
			{
				ZoneScopedN( "Render" );
				//Update
				const std::uint16_t frame_index { m_renderer.getFrameIndex() };

				const auto view_frustum { camera.getFrustumBounds() };

				FrameInfo frame_info { frame_index,
					                   delta_time,
					                   command_buffer,
					                   { camera, viewer.m_transform },
					                   global_descriptor_sets[ frame_index ],
					                   m_game_objects_root,
					                   m_renderer.getCurrentTracyCTX(),
					                   matrix_info_buffers[ frame_index ],
					                   draw_parameter_buffers[ frame_index ],
					                   m_renderer.getGBufferDescriptor( frame_index ),
					                   view_frustum };

#if TRACY_ENABLE
				//auto& tracy_ctx { frame_info.tracy_ctx };
#endif

				CameraInfo current_camera_info { .projection = camera.getProjectionMatrix(),
					                             .view = camera.getViewMatrix(),
					                             .inverse_view = camera.getInverseViewMatrix() };

				camera_info[ frame_index ] = current_camera_info;

				m_culling_system.startPass( frame_info );
				TracyVkCollect( frame_info.tracy_ctx, command_buffer );
				m_culling_system.wait();

				m_renderer.beginSwapchainRendererPass( command_buffer );

				m_terrain_system.pass( frame_info );

				m_entity_renderer.pass( frame_info );

				m_composition_system.pass( frame_info );

				gui::drawMainGUI( frame_info );

				m_renderer.endSwapchainRendererPass( command_buffer );

				m_renderer.endFrame();

				FrameMark;
			}
		}

		Device::getInstance().device().waitIdle();
	}

	void EngineContext::loadGameObjects()
	{
		ZoneScoped;
		std::cout << "Loading game objects" << std::endl;
		auto command_buffer { Device::getInstance().beginSingleTimeCommands() };

		/*
		{
			std::shared_ptr< Model > model { Model::createModel(
				Device::getInstance(),
				"models/night_heron8.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			for ( int i = 0; i < 16; ++i )
			{
				auto night_heron = GameObject::createGameObject();
				night_heron.model = model;
				night_heron.transform.translation = { -5.0f + ( i * 1.0 ), 0.5f, 0.0f };
				night_heron.transform.scale = { 0.05f, 0.05f, 0.05f };
				night_heron.transform.rotation = { 0.0f, 0.0f, 0.0f };

				night_heron.model->syncBuffers( command_buffer );

				game_objects.emplace( night_heron.getId(), std::move( night_heron ) );
			}
		}*/

		/*
		{
			std::shared_ptr< Model > model { Model::createModel(
				Device::getInstance(),
				"models/khronos-sponza/Sponza.gltf",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			assert( model );

			model->syncBuffers( command_buffer );

			constexpr int x_val { 1 };
			constexpr int y_val { x_val };

			for ( int x = 0; x < x_val; ++x )
			{
				for ( int y = 0; y < y_val; ++y )
				{
					auto sponza = GameObject::createGameObject();
					sponza.object_flags |= IS_ENTITY | IS_VISIBLE;
					sponza.m_model = model;
					sponza.m_transform.translation = WorldCoordinate( constants::WORLD_CENTER );
					// 0.0f );
					sponza.m_transform.scale = { 0.007f, 0.007f, 0.007f };
					sponza.m_transform.rotation = Rotation( 0.0f, 0.0f, 0.0f );

					m_game_objects_root.addGameObject( std::move( sponza ) );
				}
			}
		}*/

		{
			ZoneScopedN( "Load phyiscs test" );
			std::vector< std::shared_ptr< Model > > models { Model::createModelsFromScene(
				Device::getInstance(),
				"models/PhysicsTest.glb",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			for ( auto& model : models )
			{
				GameObject object { GameObject::createGameObject() };
				object.m_model = std::move( model );
				object.m_transform.translation = WorldCoordinate( 0.0f );
				object.object_flags |= IS_VISIBLE | IS_ENTITY;

				assert( object.m_model );
				object.m_model->syncBuffers( command_buffer );

				m_game_objects_root.addGameObject( std::move( object ) );
			}
		}

		{
			ZoneScopedN( "Load terrain" );
			auto model {
				generateTerrainModel( m_terrain_system.getVertexBuffer(), m_terrain_system.getIndexBuffer() )
			};

			//Texture texture { Texture::loadFromFile( "models/Vally/textures/heightmap.png" ) };
			Texture texture { Texture::generateFromPerlinNoise( 1024, 1024, 120 ) };

			Sampler sampler { vk::Filter::eLinear,
				              vk::Filter::eLinear,
				              vk::SamplerMipmapMode::eLinear,
				              vk::SamplerAddressMode::eClampToEdge };
			texture.getImageView().getSampler() = std::move( sampler );
			texture.createImGuiSet();
			Texture::getTextureDescriptorSet().bindTexture( 0, texture );
			Texture::getTextureDescriptorSet().update();

			model->m_primitives[ 0 ].m_texture = std::move( texture );

			model->syncBuffers( command_buffer );

			auto floor { GameObject::createGameObject() };
			floor.object_flags |= IS_TERRAIN | IS_VISIBLE;
			floor.m_model = model;
			assert( floor.m_model->m_primitives.size() == 1 );

			floor.m_transform.translation = WorldCoordinate( constants::WORLD_CENTER );

			m_game_objects_root.addGameObject( std::move( floor ) );
		}

		/*
		{
			std::shared_ptr< Model > model { Model::createModel(
				Device::getInstance(),
				"models/smooth_vase.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto smooth_vase = GameObject::createGameObject();
			smooth_vase.model = model;

			smooth_vase.transform.translation = { -0.5f, 0.5f, .0f };
			smooth_vase.transform.scale = { 3.0f, 1.5f, 3.0f };

			smooth_vase.model->syncBuffers( command_buffer );

			game_objects.emplace( smooth_vase.getId(), std::move( smooth_vase ) );
		}

		{
			std::shared_ptr< Model > flat_model { Model::createModel(
				Device::getInstance(),
				"models/flat_vase.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto flat_vase = GameObject::createGameObject();
			flat_vase.model = flat_model;
			//flat_vase.transform.translation = { 0.5f, 0.5f, 0.0f };
			flat_vase.transform.translation = { 0.5f, 0.5f, 0.0f };
			flat_vase.transform.scale = { 3.0f, 1.5f, 3.0f };

			flat_vase.model->syncBuffers( command_buffer );

			game_objects.emplace( flat_vase.getId(), std::move( flat_vase ) );
		}
		{
			std::shared_ptr< Model > quad_model { Model::createModel(
				Device::getInstance(),
				"models/quad.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto floor = GameObject::createGameObject();
			floor.model = quad_model;
			floor.transform.translation = { 0.0f, 0.5f, 0.0f };
			floor.transform.scale = { 3.0f, 1.0f, 3.0f };
			floor.is_world = true;

			floor.model->syncBuffers( command_buffer );

			game_objects.emplace( floor.getId(), std::move( floor ) );
		}
		 */

		Device::getInstance().endSingleTimeCommands( command_buffer );
		std::cout << "Finished loading game objects" << std::endl;

		m_game_objects_root.recalculateBoundingBoxes();
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
