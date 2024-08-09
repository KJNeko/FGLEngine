//
// Created by kj16609 on 7/7/24.
//

#pragma once

#include <cstdint>

#include "ComponentEditorInterface.hpp"

namespace fgl::engine
{

	struct ComponentEngineInterface
	{
		using ComponentID = std::uint16_t;
		virtual ComponentID id() const = 0;

		virtual ~ComponentEngineInterface() = default;
	};

} // namespace fgl::engine