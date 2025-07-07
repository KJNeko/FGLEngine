//
// Created by kj16609 on 8/13/24.
//

#pragma once

#include <chrono>

namespace fgl
{

	using Clock = std::conditional_t<
		std::chrono::high_resolution_clock::is_steady,
		std::chrono::high_resolution_clock,
		std::chrono::steady_clock >;

	using ProfilingClock = std::chrono::high_resolution_clock;

} // namespace fgl
