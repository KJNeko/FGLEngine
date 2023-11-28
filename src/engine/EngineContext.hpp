//
// Created by kj16609 on 11/27/23.
//

#pragma once

#include <memory>
#include <vector>

#include "Device.hpp"
#include "GameObject.hpp"
#include "Model.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

namespace fgl::engine
{

	class EngineContext
	{
		static constexpr int DEFAULT_WIDTH { 800 };
		static constexpr int DEFAULT_HEIGHT { 600 };

		Window m_window { DEFAULT_WIDTH, DEFAULT_HEIGHT, "titor Engine" };
		Device m_device { m_window };
		Renderer m_renderer { m_window, m_device };

		std::vector< GameObject > game_objects;

		void loadGameObjects();

	  public:

		EngineContext();
		EngineContext( EngineContext&& other ) = delete;
		EngineContext( const EngineContext& other ) = delete;
		EngineContext& operator=( const EngineContext& other ) = delete;

		void run();
	};

} // namespace fgl::engine
