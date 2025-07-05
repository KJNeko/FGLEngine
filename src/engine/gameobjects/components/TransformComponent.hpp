//
// Created by kj16609 on 3/3/25.
//
#pragma once
#include "ComponentIDS.hpp"
#include "assets/model/ModelInstanceInfo.hpp"
#include "interface/GameObjectComponent.hpp"

namespace fgl::engine
{
	class ModelInstance;
}

namespace fgl::engine::components
{
	COMPONENT_CLASS( TransformComponent, TransformComponentID )
	{
		WorldTransform m_transform;
		std::shared_ptr< ModelInstanceInfoIndex > m_model_instance_info_index;

		using Updatable = std::variant< std::weak_ptr< ModelInstance > >;

		std::vector< Updatable > m_updatables {};

		void triggerUpdate();

	  public:

		TransformComponent();
		explicit TransformComponent( const WorldTransform& transform );

		void drawImGui() override;

		std::string_view humanName() const override;
		std::string_view className() const override;

		WorldTransform& operator*();

		void setTransform( const WorldTransform& transform );

		~TransformComponent() override;

		template < typename T >
		void addUpdateTarget( const T& unique )
		{
			// static_assert( std::constructible_from< Updatable, T >, "T must be Updatable" );

			m_updatables.emplace_back( unique );
			triggerUpdate();
		}
	};

} // namespace fgl::engine::components
