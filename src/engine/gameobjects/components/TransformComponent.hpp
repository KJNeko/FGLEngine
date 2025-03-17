//
// Created by kj16609 on 3/3/25.
//
#pragma once
#include "ComponentIDS.hpp"
#include "interface/GameObjectComponent.hpp"

namespace fgl::engine::components
{

	COMPONENT_CLASS( TransformComponent, TransformComponentID )
	{
		WorldTransform m_transform;

	  public:

		explicit TransformComponent( WorldTransform & transform );

		void drawImGui() override;

		std::string_view humanName() const override;
		std::string_view className() const override;

		WorldTransform& operator*()
		{
			return m_transform;
		}

		virtual ~TransformComponent() override = default;
	};

} // namespace fgl::engine::components
