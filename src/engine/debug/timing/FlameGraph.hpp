//
// Created by kj16609 on 10/29/24.
//

#pragma once

#include <string_view>

namespace fgl::engine::debug
{

	namespace timing
	{

		struct ScopedTimer;

		void reset();
		[[nodiscard]] ScopedTimer push( std::string_view name );

		namespace internal
		{
			void pop();
		}

		void render();

		struct ScopedTimer
		{
			~ScopedTimer() { internal::pop(); }
		};

	} // namespace timing

} // namespace fgl::engine::debug