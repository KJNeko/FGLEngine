//
// Created by kj16609 on 7/30/24.
//
#include <cstdlib>

#include "engine/EngineContext.hpp"
#include "engine/camera/CameraManager.hpp"
#include "engine/gameobjects/components/CameraComponent.hpp"
#include "gui/core.hpp"

int main()
{
	using namespace fgl::engine;

	log::set_level( spdlog::level::debug );

	EngineContext engine_ctx {};

	// We start by hooking into the imgui rendering.
	engine_ctx.hookInitImGui( gui::initGui );
	engine_ctx.hookCleanupImGui( gui::cleanupImGui );
	engine_ctx.TEMPhookGuiRender( gui::drawMainGUI );

	// Now we need to create the camera for the editor.
	CameraManager& camera_manager { CameraManager::getInstance() };

	auto& editor_camera { camera_manager.getPrimary() };

	editor_camera->setFOV( glm::radians( 90.0f ) );

	//! Will be true until the window says it wants to close.
	while ( engine_ctx.good() )
	{
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
	}

	engine_ctx.run();

	return EXIT_SUCCESS;
}
