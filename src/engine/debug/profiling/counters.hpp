//
// Created by kj16609 on 8/8/24.
//

#pragma once

#include <cstdint>

namespace fgl::engine::profiling
{
	struct Counters
	{
		std::size_t verts_drawn;
		std::size_t models_draw;
	};

	Counters& getCounters();

	void addModelDrawn( std::size_t n = 1 );
	void addVertexDrawn( std::size_t n );

	void resetCounters();

} // namespace fgl::engine::profiling
