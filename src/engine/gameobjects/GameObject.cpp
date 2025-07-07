//
// Created by kj16609 on 11/30/23.
//

#include "GameObject.hpp"

namespace fgl::engine
{

	GameObject::~GameObject()
	{
		if ( m_id != INVALID_ID )
		{
			log::debug( "Destroyed game object {}", this->m_id );
			for ( const auto& component : m_components ) delete component;
		}
	}

	GameObject& GameObject::operator=( GameObject&& other ) noexcept
	{
		m_id = other.m_id;
		m_object_flags = other.m_object_flags;
		m_components = std::move( other.m_components );
		m_name = std::move( other.m_name );

		other.m_id = INVALID_ID;

		return *this;
	}

	GameObject::GameObject( GameObject&& other ) noexcept :
	  m_id( other.m_id ),
	  m_object_flags( other.m_object_flags ),
	  m_components( std::move( other.m_components ) ),
	  m_name( other.m_name )
	{
		other.m_id = INVALID_ID;
	}

	GameObject GameObject::createGameObject()
	{
		static GameObjectID current_id { 0 };
		return GameObject( current_id++ );
	}

} // namespace fgl::engine