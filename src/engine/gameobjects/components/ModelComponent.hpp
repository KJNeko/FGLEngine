//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <memory>

#include "ComponentIDS.hpp"
#include "TransformComponent.hpp"
#include "assets/model/ModelInstance.hpp"
#include "interface/GameObjectComponent.hpp"

namespace fgl::engine
{
	class Model;
	class ModelInstance;
	class ModelRenderHandle;
} // namespace fgl::engine

namespace fgl::engine::components
{
	struct GameModel;

	COMPONENT_CLASS( ModelComponent, ModelComponentID )
	{
		std::shared_ptr< ModelInstance > m_model_instance;
		TransformComponent m_transform {};
		// In the future this should also contain a handle to the pipeline that is going to be used for rendering this model.

	  public:

		ModelComponent() = delete;
		explicit ModelComponent( const std::shared_ptr< Model >& model );

		void drawImGui() override;

		std::string_view humanName() const override;
		std::string_view className() const override;

		virtual ~ModelComponent() override
		{}

		TransformComponent getTransform() const
		{
			return m_transform;
		}
		void updateTransform( const TransformComponent& transform )
		{
			m_transform = transform;
		}

		// Model* operator->();

		// const Model* operator->() const;
	};

	static_assert( is_component< ModelComponent > );

} // namespace fgl::engine::components
