//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <filesystem>
#include <memory>

#include "GameObjectComponent.hpp"
#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{
	class Model;

	class ModelComponent final : public GameObjectComponent< 1 >
	{
		std::shared_ptr< Model > m_model;
		TransformComponent m_model_transform {};

	  public:

		ModelComponent( std::shared_ptr< Model >&& model ) : m_model( std::forward< decltype( m_model ) >( model ) ) {}

		void drawImGui() override {}

		std::string_view name() const override {}

		virtual ~ModelComponent() override {}

		void setModel( const std::filesystem::path path );

		Model* operator->() { return m_model.get(); }

		const Model* operator->() const { return m_model.get(); }
	};

	static_assert( is_component< ModelComponent > );

} // namespace fgl::engine
