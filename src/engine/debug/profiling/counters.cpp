//
// Created by kj16609 on 8/8/24.
//

#include "counters.hpp"

#include <cstring>
#include <memory>

namespace fgl::engine::profiling
{
	inline static Counters counters;

	Counters& getCounters()
	{
		return counters;
	}

	void addModelDrawn( std::size_t n )
	{
		counters.models_draw += n;
	}

	void addVertexDrawn( std::size_t n )
	{
		counters.verts_drawn += n;
	}

	void resetCounters()
	{
		counters.verts_drawn = 0;
		counters.models_draw = 0;
	}

	// In order for resetCounters to work we need to ensure we can just zero the struct.
	// It being trivially copyable fits this requirement
	static_assert( std::is_trivially_copyable_v< Counters > );
	static_assert( std::is_trivially_default_constructible_v< Counters > );

} // namespace fgl::engine::profiling
