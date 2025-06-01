//
// Created by kj16609 on 7/30/24.
//
#include <cstdlib>

#include "assets/model/builders/SceneBuilder.hpp"
#include "debug/profiling/counters.hpp"
#include "engine/EngineContext.hpp"
#include "engine/camera/CameraManager.hpp"
#include "engine/debug/timing/FlameGraph.hpp"
#include "engine/gameobjects/components/CameraComponent.hpp"
#include "gui/EditorGuiContext.hpp"

int main()
{
	using namespace fgl::engine;
	using namespace fgl::editor;

	log::set_level( spdlog::level::debug );

	const auto version { vk::enumerateInstanceVersion() };

	// variant 3 bit int, bits 31-29
	// major 7-bit, bits 28-22
	// minor, 10 bit, 21-12
	// patch, 12 bit, 10-0
	// clang-format off
	constexpr std::uint64_t PATCH_BITMASK	{ 0b00000000000000000000111111111111 };
	constexpr std::uint64_t MINOR_BITMASK	{ 0b00000000001111111111000000000000 };
	constexpr std::uint64_t MAJOR_BITMASK	{ 0b00011111110000000000000000000000 };
	constexpr std::uint64_t VARIANT_BITMASK { 0b11100000000000000000000000000000 };

	const auto patch 	{ ( version & PATCH_BITMASK ) 	>> 0};
	const auto minor 	{ ( version & MINOR_BITMASK ) 	>> 10};
	const auto major 	{ ( version & MAJOR_BITMASK ) 	>> (10 + 12)};
	[[maybe_unused]] const auto variant 	{ ( version & VARIANT_BITMASK ) >> (10 + 12 + 7) };

	// clang-format on

	log::debug( "Vulkan instance version: {}.{}.{}.{}", major, minor, patch, minor );

	try
	{
		EngineContext engine_ctx {};

		EditorGuiContext editor_ctx { engine_ctx.getWindow() };

		// We start by hooking into the imgui rendering.
		engine_ctx.hookPreFrame(
			[ & ]( [[maybe_unused]] FrameInfo& info )
			{
				editor_ctx.beginDraw();

				profiling::resetCounters();
			} );
		engine_ctx.hookLateFrame( [ & ]( [[maybe_unused]] FrameInfo& info ) { editor_ctx.draw( info ); } );
		engine_ctx.hookLateFrame( [ & ]( FrameInfo& info ) { editor_ctx.endDraw( info ); } );

		// Now we need to create the camera for the editor.
		CameraManager& camera_manager { engine_ctx.cameraManager() };

		auto& editor_camera { camera_manager.getPrimary() };

		editor_camera->setFOV( glm::radians( 90.0f ) );

		// Create a default world to assign to the engine before we load or create a new one.
		// World world {};

		[[maybe_unused]] constexpr bool playing { false };

		constexpr bool preload { true };

		if ( preload )
		{
			auto& buffers { getModelBuffers() };
			SceneBuilder builder { buffers.m_vertex_buffer, buffers.m_index_buffer };

			constexpr std::string_view sponza_path {
				// "/home/kj16609/Desktop/Projects/cxx/Mecha/src/assets/PBRSphere.gltf"
				"/home/kj16609/Desktop/Projects/cxx/Mecha/src/assets/khronos-sponza/Sponza.gltf"
			};

			builder.loadScene( sponza_path );

			std::vector< GameObject > objs { builder.getGameObjects() };

			for ( auto& obj : objs )
			{
				if ( obj.hasComponent< components::ModelComponent >() )
				{
					auto model_components { obj.getComponents< components::ModelComponent >() };

					for ( [[maybe_unused]] auto& component : model_components )
					{}
				}

				engine_ctx.game_objects.emplace_back( std::move( obj ) );
			}
		}

		//! Will be true until the window says it wants to close.
		while ( engine_ctx.good() )
		{
			debug::timing::reset();

			engine_ctx.tickDeltaTime();

			// engine_ctx.setWorld( world );

			// if ( playing ) world = engine_ctx.tickSimulation();

			engine_ctx.handleTransfers();

			// Process input
			engine_ctx.processInput();

			// Here we can decide if we want to tick fully or not.

			// Simulate step
			// engine_ctx.tickSimulation();

			// Update the viewer camera

			// Render step
			engine_ctx.renderFrame();

			engine_ctx.finishFrame();
			// This will 'end' the root node, Which is created on 'reset'
			debug::timing::internal::pop();
		}

		engine_ctx.waitIdle();
	}
	catch ( const vk::LayerNotPresentError& e )
	{
		log::info( "{}:{}", e.code().message(), e.what() );
	}

	return EXIT_SUCCESS;
}
