//
// Created by kj16609 on 7/30/24.
//
#include <cstdlib>

#include "engine/EngineContext.hpp"
#include "engine/camera/CameraManager.hpp"
#include "engine/debug/timing/FlameGraph.hpp"
#include "engine/gameobjects/components/CameraComponent.hpp"
#include "gui/core.hpp"

int main()
{
	using namespace fgl::engine;

	log::set_level( spdlog::level::debug );

	EngineContext engine_ctx {};

	// We start by hooking into the imgui rendering.
	engine_ctx.hookInitImGui( gui::initGui );
	engine_ctx.hookPreFrame( gui::startDrawImGui );
	engine_ctx.hookEarlyFrame( gui::drawImGui );
	engine_ctx.hookLateFrame( gui::endDrawImGui );
	engine_ctx.hookDestruction( gui::cleanupImGui );

	// Now we need to create the camera for the editor.
	CameraManager& camera_manager { engine_ctx.cameraManager() };

	auto& editor_camera { camera_manager.getPrimary() };

	editor_camera->setFOV( glm::radians( 90.0f ) );

	//! Will be true until the window says it wants to close.
	while ( engine_ctx.good() )
	{
		debug::timing::reset();
		engine_ctx.tickDeltaTime();

		engine_ctx.handleTransfers();

		// Process input
		engine_ctx.processInput();

		// Here we can decide if we want to tick fully or not.

		// Simulate step
		engine_ctx.tickSimulation();

		// Update the viewer camera

		// Render step
		engine_ctx.renderFrame();

		engine_ctx.finishFrame();
		// This will 'end' the root node, Which is created on 'reset'
		debug::timing::internal::pop();
	}

	return EXIT_SUCCESS;
}
