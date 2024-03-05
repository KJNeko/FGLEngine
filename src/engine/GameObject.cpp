//
// Created by kj16609 on 11/30/23.
//

#include "GameObject.hpp"

namespace fgl::engine
{

	GameObject GameObject::createGameObject()
	{
		static ID current_id { 0 };
		return { current_id++ };
	}

} // namespace fgl::engine