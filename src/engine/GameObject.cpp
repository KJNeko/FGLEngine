//
// Created by kj16609 on 11/30/23.
//

#include "GameObject.hpp"

#include "engine/model/Model.hpp"

namespace fgl::engine
{

	GameObject GameObject::createGameObject()
	{
		static ID current_id { 0 };
		return { current_id++ };
	}

	OrientedBoundingBox< CoordinateSpace::World > GameObject::getBoundingBox() const
	{
		return this->m_transform.mat() * this->m_model->getBoundingBox();
	}

} // namespace fgl::engine