//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <filesystem>
#include <memory>

#include "GameObjectComponent.hpp"

namespace fgl::engine
{
	class Model;

	class ModelComponent final : public GameObjectComponent< 1 >
	{
		std::shared_ptr< Model > m_model;

	  public:

		ModelComponent( std::shared_ptr< Model >&& model ) : m_model( std::forward< decltype( m_model ) >( model ) ) {}

#ifdef TITOR_EDITOR
		void drawImGui() override;

		std::string_view name() const override
		{
			//TODO: Get name of component
			return "TEST NAME";
		}
#endif

		virtual ~ModelComponent() override {}

		Model* operator->() { return m_model.get(); }

		const Model* operator->() const { return m_model.get(); }
	};

	static_assert( is_component< ModelComponent > );

} // namespace fgl::engine
