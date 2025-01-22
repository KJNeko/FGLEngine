//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include "Window.hpp"
#include "camera/CameraManager.hpp"
#include "clock.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/rendering/Renderer.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
#include "systems/composition/GuiSystem.hpp"

namespace fgl::engine
{
	class CameraManager;
	class GameObject;

	using namespace fgl::literals::size_literals;

	inline void dummyFrameInfoFunc( [[maybe_unused]] FrameInfo& frame_info )
	{
		return;
	}

	using FrameHookFunc = std::function< void( FrameInfo& ) >;

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

		std::unique_ptr< memory::Buffer > m_vertex_buffer { std::make_unique< memory::Buffer >(
			1_GiB,
			vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal ) };
		std::unique_ptr< memory::Buffer > m_index_buffer { std::make_unique< memory::Buffer >(
			512_MiB,
			vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eDeviceLocal ) };

		memory::TransferManager m_transfer_manager { device, 128_MiB };

		//GameObject::Map game_objects {};
		OctTreeNode m_game_objects_root { WorldCoordinate( constants::WORLD_CENTER ) };

		// SubPass 0
		GuiSystem m_gui_system {};

		// Functions BEFORE a frame is started
		std::vector< FrameHookFunc > pre_frame_hooks {};

		//! TODO: Make this so we can tell at what stage we should be doing something
		std::vector< FrameHookFunc > early_render_hooks {};
		std::vector< FrameHookFunc > render_hooks {};
		std::vector< FrameHookFunc > late_render_hooks {};

		// Functions to call upon the frame ending (This happens AFTER the GPU call is dispatched)
		std::vector< FrameHookFunc > post_frame_hooks {};

		//! Called before the context is destroyed
		std::vector< std::function< void() > > destruction_hooks {};

		// Memory pool for shader uniforms.
		memory::Buffer m_ubo_buffer_pool;

		memory::Buffer m_material_data_pool;

		// Memory pool for matrix info and draw parameters
		memory::Buffer m_matrix_info_pool;
		memory::Buffer m_draw_parameter_pool;

		CameraManager m_camera_manager {};

		std::chrono::time_point< fgl::Clock > last_tick { fgl::Clock::now() };
		double m_delta_time;

	  public:

		FGL_FORCE_INLINE_FLATTEN void hookInitImGui( const std::function< void( Window&, Renderer& ) >& func )
		{
			func( m_window, m_renderer );
		}

		void hookPreFrame( const FrameHookFunc& func ) { pre_frame_hooks.emplace_back( func ); }

		void hookEarlyFrame( const FrameHookFunc& func ) { early_render_hooks.emplace_back( func ); }

		void hookFrame( const FrameHookFunc& func ) { render_hooks.emplace_back( func ); }

		void hookLateFrame( const FrameHookFunc& func ) { late_render_hooks.emplace_back( func ); }

		void hookPostFrame( const FrameHookFunc& func ) { post_frame_hooks.emplace_back( func ); }

		void hookDestruction( const std::function< void() >& func ) { destruction_hooks.emplace_back( func ); }

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

		//! Runs any post-frame processes
		void finishFrame();

		//! Waits for all frames to be completed
		void waitIdle();

		Window& getWindow();
		float getWindowAspectRatio();

		CameraManager& cameraManager();
	};

} // namespace fgl::engine
