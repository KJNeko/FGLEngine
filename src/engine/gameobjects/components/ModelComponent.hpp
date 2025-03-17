//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <memory>

#include "ComponentIDS.hpp"
#include "assets/model/ModelRenderHandle.hpp"
#include "engine/gameobjects/components/interface/GameObjectComponent.hpp"

namespace fgl::engine
{
	class ModelRenderHandle;
}

namespace fgl::engine::components
{
	struct GameModel;
	class Model;

	COMPONENT_CLASS( ModelComponent, ModelComponentID )
	{
		std::shared_ptr< Model > m_model;
		ModelRenderHandle m_render_handle;
		// In the future this should also contain a handle to the pipeline that is going to be used for rendering this model.

	  public:

		explicit ModelComponent( std::shared_ptr< Model > && model );

		void drawImGui() override;

		std::string_view humanName() const override;
		std::string_view className() const override;

		virtual ~ModelComponent() override
		{}

		Model* operator->();

		const Model* operator->() const;
	};

	static_assert( is_component< ModelComponent > );

} // namespace fgl::engine::components
