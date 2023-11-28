//
// Created by kj16609 on 11/27/23.
//

#include "EngineContext.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ON
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>
#include <chrono>
#include <thread>

#include "RenderSystem.hpp"
#include "createCube.cpp"
#include "engine/debug_gui/DebugGUI.hpp"

namespace fgl::engine
{
	void EngineContext::run()
	{
		RenderSystem render_system { m_device, m_renderer.getSwapChainRenderPass() };
		Camera camera {};

		while ( !m_window.shouldClose() )
		{
			glfwPollEvents();

			const float aspect { m_renderer.getAspectRatio() };

			//camera.setOrthographicProjection( -aspect, aspect, -1, 1, -1, 1 );
			camera.setPerspectiveProjection( glm::radians( 50.0f ), aspect, 0.1f, 10.f );

			if ( auto command_buffer = m_renderer.beginFrame(); command_buffer )
			{
				m_renderer.beginSwapchainRendererPass( command_buffer );

				render_system.renderGameObjects( command_buffer, game_objects, camera );

				m_renderer.endSwapchainRendererPass( command_buffer );
				m_renderer.endFrame();
			}

			using namespace std::chrono_literals;
			std::this_thread::sleep_for( 16.66ms );
		}

		vkDeviceWaitIdle( m_device.device() );
	}

	void EngineContext::loadGameObjects()
	{
		std::shared_ptr< Model > model { createCubeModel( m_device, { 0.0f, 0.0f, 0.0f } ) };

		auto cube = GameObject::createGameObject();
		cube.model = model;
		cube.transform.translation = { 0.0f, 0.0f, 2.0f };
		cube.transform.scale = { 0.5f, 0.5f, 0.5f };

		game_objects.push_back( std::move( cube ) );
	}

	EngineContext::EngineContext()
	{
		loadGameObjects();

		debug::initDebugGUI();
	}

} // namespace fgl::engine