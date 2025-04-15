//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include "Window.hpp"
#include "assets/MaterialManager.hpp"
#include "assets/model/Model.hpp"
#include "camera/CameraManager.hpp"
#include "clock.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/rendering/Renderer.hpp"
#include "scene/World.hpp"
#include "systems/composition/GuiSystem.hpp"

namespace fgl::engine
{
	class CameraManager;
	class GameObject;

	using namespace fgl::literals::size_literals;

	inline void dummyFrameInfoFunc( [[maybe_unused]] FrameInfo& frame_info )
	{}

	using FrameHookFunc = std::function< void( FrameInfo& ) >;

	class EngineContext
	{
		static constexpr int DEFAULT_WIDTH { 1920 };
		static constexpr int DEFAULT_HEIGHT { 1080 };

		vk::raii::Context m_ctx {};

		// Window must be prepared *BEFORE* instance is ready in order to make
		// glfwGetRequiredInstanceExtensions valid
		Window m_window { DEFAULT_WIDTH, DEFAULT_HEIGHT, "titor Engine" };

		Instance m_instance { m_ctx };

		Device m_device { m_window, m_instance };

		Renderer m_renderer { m_window, m_device.phyDevice() };

		// GameObject::Map game_objects {};
		// OctTreeNode m_game_objects_root { WorldCoordinate( constants::WORLD_CENTER ) };

		// SubPass 0
		GuiSystem m_gui_system {};

		// Functions BEFORE a frame is started
		std::vector< FrameHookFunc > m_pre_frame_hooks {};

		//! TODO: Make this so we can tell at what stage we should be doing something
		std::vector< FrameHookFunc > m_early_render_hooks {};
		std::vector< FrameHookFunc > m_render_hooks {};
		std::vector< FrameHookFunc > m_late_render_hooks {};

		// Functions to call upon the frame ending (This happens AFTER the GPU call is dispatched)
		std::vector< FrameHookFunc > m_post_frame_hooks {};

		//! Called before the context is destroyed
		std::vector< std::function< void() > > m_destruction_hooks {};

		// Memory pool for shader uniforms.
		memory::Buffer m_ubo_buffer_pool;

		// Memory pool for matrix info and draw parameters
		memory::Buffer m_draw_parameter_pool;

	  public:

		std::vector< GameObject > game_objects {};

		ModelGPUBuffers m_model_buffers {};

	  private:

		PerFrameArray< DeviceVector< vk::DrawIndexedIndirectCommand > > m_gpu_draw_commands;
		//TODO: Outright remove this. Or the one in model buffers.
		PerFrameArray< DeviceVector< PerVertexInstanceInfo > >& m_per_vertex_infos;
		PerFrameArray< std::unique_ptr< descriptors::DescriptorSet > > m_gpu_draw_cmds_desc;

		MaterialManager m_material_manager {};

		CameraManager m_camera_manager {};

		memory::TransferManager m_transfer_manager { m_device, 32_MiB };

		std::chrono::time_point< Clock > m_last_tick { Clock::now() };
		DeltaTime m_delta_time;

		// World m_world;

	  public:

		// ModelManager& models() { return m_model_manager; }

		FGL_FORCE_INLINE_FLATTEN void hookInitImGui( const std::function< void( Window&, Renderer& ) >& func )
		{
			func( m_window, m_renderer );
		}

		void hookPreFrame( const FrameHookFunc& func ) { m_pre_frame_hooks.emplace_back( func ); }

		void hookEarlyFrame( const FrameHookFunc& func ) { m_early_render_hooks.emplace_back( func ); }

		void hookFrame( const FrameHookFunc& func ) { m_render_hooks.emplace_back( func ); }

		void hookLateFrame( const FrameHookFunc& func ) { m_late_render_hooks.emplace_back( func ); }

		void hookPostFrame( const FrameHookFunc& func ) { m_post_frame_hooks.emplace_back( func ); }

		void hookDestruction( const std::function< void() >& func ) { m_destruction_hooks.emplace_back( func ); }

		void setWorld( const World& world );

	  public:

		EngineContext();
		~EngineContext();

		static EngineContext& getInstance();

		bool good();

		//! Performs and pending memory transfers
		void handleTransfers();

		EngineContext( EngineContext&& other ) = delete;
		EngineContext( const EngineContext& other ) = delete;
		EngineContext& operator=( const EngineContext& other ) = delete;

		void processInput();

		void tickDeltaTime();
		World tickSimulation();
		void renderCameras( FrameInfo frame_info );

		void renderFrame();

		//! Runs any post-frame processes
		void finishFrame();

		//! Waits for all frames to be completed
		void waitIdle();

		Window& getWindow();
		float getWindowAspectRatio();

		CameraManager& cameraManager();
		MaterialManager& getMaterialManager();
	};

} // namespace fgl::engine
