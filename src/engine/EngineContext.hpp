//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include <memory>
#include <vector>

#include "Device.hpp"
#include "GameObject.hpp"
#include "Renderer.hpp"
#include "Window.hpp"
#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/model/Model.hpp"
#include "engine/systems/CullingSystem.hpp"
#include "engine/systems/EntityRendererSystem.hpp"

namespace fgl::engine
{

	class EngineContext
	{
		static constexpr int DEFAULT_WIDTH { 1920 };
		static constexpr int DEFAULT_HEIGHT { 1080 };

		Window m_window { DEFAULT_WIDTH, DEFAULT_HEIGHT, "titor Engine" };
		Renderer m_renderer { m_window };

		GameObject::Map game_objects {};

		CullingSystem m_culling_system {};
		EntityRendererSystem m_entity_renderer { Device::getInstance(), m_renderer.getSwapChainRenderPass() };

		void loadGameObjects();

		void initImGui();
		void cleanupImGui();

	  public:

		EngineContext();
		~EngineContext();
		EngineContext( EngineContext&& other ) = delete;
		EngineContext( const EngineContext& other ) = delete;
		EngineContext& operator=( const EngineContext& other ) = delete;

		void run();
	};

} // namespace fgl::engine
