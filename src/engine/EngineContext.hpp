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

		void loadGameObjects();

		void initImGui();

	  public:

		EngineContext();
		~EngineContext();
		EngineContext( EngineContext&& other ) = delete;
		EngineContext( const EngineContext& other ) = delete;
		EngineContext& operator=( const EngineContext& other ) = delete;

		void run();
	};

} // namespace fgl::engine
