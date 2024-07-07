//
// Created by kj16609 on 7/7/24.
//

#pragma once

namespace fgl::engine
{

	struct GameObjectComponentBase
	{
		using ComponentID = std::uint8_t;
		virtual ComponentID id() const = 0;
		virtual std::string_view name() const = 0;

		virtual ~GameObjectComponentBase() = default;
	};

} // namespace fgl::engine