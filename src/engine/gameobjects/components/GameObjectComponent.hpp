//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include "ComponentImGuiInterface.hpp"
#include "GameObjectComponentBase.hpp"

namespace fgl::engine
{

	template < GameObjectComponentBase::ComponentID T_ID >
	struct GameObjectComponent : ComponentImGuiInterface, GameObjectComponentBase
	{
		constexpr static ComponentID ID { T_ID };

		virtual ComponentID id() const override final { return ID; }
	};

	template < typename T >
	concept is_component = requires( T t ) {
		std::is_base_of_v< T, GameObjectComponentBase >;
		{
			t.ID
		} -> std::same_as< GameObjectComponentBase::ComponentID >;
	};

} // namespace fgl::engine