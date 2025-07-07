//
// Created by kj16609 on 8/8/24.
//

#include "counters.hpp"

#include <cstring>
#include <memory>

namespace fgl::engine::profiling
{
	inline static Counters COUNTERS;

	Counters& getCounters()
	{
		return COUNTERS;
	}

	void addModelDrawn( std::size_t n )
	{
		COUNTERS.m_models_draw += n;
	}

	void addVertexDrawn( std::size_t n )
	{
		COUNTERS.m_verts_drawn += n;
	}

	void addInstances( std::size_t n )
	{
		COUNTERS.m_instance_count += n;
	}

	void resetCounters()
	{
		COUNTERS.m_verts_drawn = 0;
		COUNTERS.m_models_draw = 0;
		COUNTERS.m_instance_count = 0;
	}

	// In order for resetCounters to work we need to ensure we can just zero the struct.
	// It being trivially copyable fits this requirement
	static_assert( std::is_trivially_copyable_v< Counters > );
	static_assert( std::is_trivially_default_constructible_v< Counters > );

} // namespace fgl::engine::profiling
