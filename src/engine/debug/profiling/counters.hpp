//
// Created by kj16609 on 8/8/24.
//

#pragma once

#include <cstdint>

namespace fgl::engine::profiling
{
	struct Counters
	{
		std::size_t m_verts_drawn;
		std::size_t m_models_draw;
		std::size_t m_instance_count;
	};

	Counters& getCounters();

	void addModelDrawn( std::size_t n = 1 );
	void addVertexDrawn( std::size_t n );
	void addInstances( std::size_t n = 1 );

	void resetCounters();

} // namespace fgl::engine::profiling
