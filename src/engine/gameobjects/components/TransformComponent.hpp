//
// Created by kj16609 on 3/3/25.
//
#pragma once
#include "ComponentIDS.hpp"
#include "assets/model/ModelInstanceInfo.hpp"
#include "interface/GameObjectComponent.hpp"

namespace fgl::engine::components
{

	COMPONENT_CLASS( TransformComponent, TransformComponentID )
	{
		WorldTransform m_transform;
		std::shared_ptr< ModelInstanceInfoIndex > m_model_instance_info_index;

	  public:

		TransformComponent();
		explicit TransformComponent( const WorldTransform& transform );

		void drawImGui() override;

		std::string_view humanName() const override;
		std::string_view className() const override;

		WorldTransform& operator*();

		virtual ~TransformComponent() override
		{}
	};

} // namespace fgl::engine::components
