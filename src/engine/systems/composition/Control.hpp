//
// Created by kj16609 on 10/18/24.
//

#pragma once

#include <cstdint>

namespace fgl::engine
{
	struct CompositionControl
	{
		enum Values
		{
			OutColor = 0,
			LoRender = 1,
			Normal = 2,
			CosLo = 3,
			F0 = 4,
			DirectLighting = 5,
			Ambient = 6,
			Lh = 7,
			Max
		};

		int m_flags { 0 };
	};
} // namespace fgl::engine
