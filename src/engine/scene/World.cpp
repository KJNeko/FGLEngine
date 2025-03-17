//
// Created by kj16609 on 3/2/25.
//
#include "World.hpp"

namespace fgl::engine
{

	bool World::sceneExists( const std::string& name )
	{
		return std::ranges::
		           find_if( m_scenes, [ &name ]( const auto& scene ) -> bool { return scene.getName() == name; } )
		    != m_scenes.end();
	}

	std::shared_ptr< Scene > World::createScene()
	{
		constexpr std::string default_scene { "Scene" };

		std::string name { default_scene };

		// keep finding new name until we find one not used yet.
		if ( sceneExists( name ) )
		{
			std::uint8_t i { 1 };
			name = std::format( "Scene {}", i );

			while ( sceneExists( name ) )
			{
				name = std::format( "Scene {}", ++i );
			}
		}

		auto ptr { std::make_shared< Scene >( name ) };

		m_scenes.push_back( ptr );

		return ptr;
	}

	World::World( const std::filesystem::path& path )
	{}

} // namespace fgl::engine
