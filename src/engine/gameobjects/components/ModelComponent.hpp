//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <memory>

#include "ComponentIDS.hpp"
#include "engine/gameobjects/components/interface/GameObjectComponent.hpp"

namespace fgl::engine
{
	class Model;

	COMPONENT_CLASS( ModelComponent, ModelComponentID )
	{
		std::shared_ptr< Model > m_model;

	  public:

		ModelComponent( std::shared_ptr< Model > && model ) : m_model( std::forward< decltype( m_model ) >( model ) )
		{}

		void drawImGui() override;

		std::string_view name() const override;

		virtual ~ModelComponent() override
		{}

		Model* operator->();

		const Model* operator->() const;
	};

	static_assert( is_component< ModelComponent > );

} // namespace fgl::engine
