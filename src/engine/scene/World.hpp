//
// Created by kj16609 on 3/2/25.
//
#pragma once

#include <vector>

#include "Scene.hpp"

namespace fgl::engine
{

	/** A world is the highest representation of scenes in an engine,
	* a world is comprised up of multiple scenes. Only one world can be running at any given time
	*
	*/
	class World
	{
		std::vector< std::shared_ptr< Scene > > m_scenes {};

		std::shared_ptr< ModelManager > m_model_manager { std::make_shared< ModelManager >() };

	  public:

		bool sceneExists( const std::string& name );

		std::shared_ptr< Scene > createScene();

		FGL_DEFAULT_ALL_RO5( World );
		explicit World( const std::filesystem::path& path );
		explicit World( std::vector< std::byte >& data );

		//! Dumps the world save info to a vector
		std::vector< std::byte > dump();

		~World() = default;
	};

} // namespace fgl::engine