//
// Created by kj16609 on 7/7/24.
//

#pragma once
#include <memory>

#include "GameObjectComponent.hpp"

namespace fgl::engine
{
	class Model;

	class ModelComponent final : public GameObjectComponent< 1 >
	{
		std::shared_ptr< Model > m_model;

	  public:

		void drawImGui() override;
		std::string_view name() const override;
		~ModelComponent() override;
	};

} // namespace fgl::engine
