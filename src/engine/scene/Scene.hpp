//
// Created by kj16609 on 2/28/25.
//
#pragma once
#include "camera/Camera.hpp"

namespace fgl::engine
{
	class GameObject;
	class World;

	class Scene
	{
		std::string m_name {};

		std::vector< std::shared_ptr< GameObject > > m_objects {};

	  public:

		Scene() = delete;

		FGL_DELETE_MOVE( Scene );
		FGL_DELETE_COPY( Scene );

		std::string& getName() { return m_name; };

		const std::string& getName() const { return m_name; };

		//!! Loads a scene from a gltf file
		Scene( const std::string& name );
	};

} // namespace fgl::engine