//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include "Window.hpp"
#include "engine/literals/size.hpp"
#include "engine/rendering/Renderer.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
#include "systems/GuiSystem.hpp"
#include "systems/TerrainSystem.hpp"

namespace fgl::engine
{
	class CameraManager;
	class GameObject;

	using namespace fgl::literals::size_literals;

	class EngineContext
	{
		static constexpr int DEFAULT_WIDTH { 1920 };
		static constexpr int DEFAULT_HEIGHT { 1080 };

		vk::raii::Context ctx {};

		// Window must be prepared *BEFORE* instance is ready in order to make
		// glfwGetRequiredInstanceExtensions valid
		Window m_window { DEFAULT_WIDTH, DEFAULT_HEIGHT, "titor Engine" };

		Instance m_instance { ctx };

		Device device { m_window, m_instance };

		Renderer m_renderer { m_window, device.phyDevice() };

		//GameObject::Map game_objects {};
		OctTreeNode m_game_objects_root { WorldCoordinate( constants::WORLD_CENTER ) };

		std::unique_ptr< memory::Buffer > m_vertex_buffer { std::make_unique< memory::Buffer >(
			256_MiB,
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal ) };
		std::unique_ptr< memory::Buffer > m_index_buffer { std::make_unique< memory::Buffer >(
			32_MiB,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal ) };

		// SubPass 0
		GuiSystem m_gui_system { Device::getInstance(), m_renderer.getSwapChainRenderPass() };

		// Temp function
		std::function< void( FrameInfo& ) > renderGui;
		std::function< void() > cleanupImGui;

		// Memory pool for shader uniforms.
		memory::Buffer m_ubo_buffer_pool;

		// Memory pool for matrix info and draw parameters
		memory::Buffer m_matrix_info_pool;
		memory::Buffer m_draw_parameter_pool;

		std::chrono::time_point< std::chrono::high_resolution_clock > last_tick {
			std::chrono::high_resolution_clock::now()
		};
		double m_delta_time_ms;

		void loadGameObjects();

#ifdef IDHAN_EDITOR

	  public:

#endif

		FGL_FORCE_INLINE_FLATTEN void hookInitImGui( const std::function< void( Window&, Renderer& ) >& func )
		{
			func( m_window, m_renderer );
		}

		FGL_FORCE_INLINE_FLATTEN void hookCleanupImGui( const std::function< void() >& func ) { cleanupImGui = func; }

		void TEMPhookGuiRender( const std::function< void( FrameInfo& ) >& func ) { renderGui = func; }

	  public:

		EngineContext();
		~EngineContext();

		bool good();

		//! Performs and pending memory transfers
		void handleTransfers();

		EngineContext( EngineContext&& other ) = delete;
		EngineContext( const EngineContext& other ) = delete;
		EngineContext& operator=( const EngineContext& other ) = delete;

		void processInput();

		void tickDeltaTime();
		void tickSimulation();
		void renderCameras( FrameInfo frame_info );

		void renderFrame();

		Window& getWindow();
		float getWindowAspectRatio();

		void run();
	};

} // namespace fgl::engine
