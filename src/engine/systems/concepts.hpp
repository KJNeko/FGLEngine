//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include <concepts>

namespace vk
{
	class CommandBuffer;
}

namespace fgl::engine
{

	struct FrameInfo;

	template < typename T >
	concept is_system = requires( T t, FrameInfo info ) {
		{
			t.pass( info )
		};
	};

	template < typename T >
	concept is_threaded_system = requires( T t, FrameInfo info ) {
		requires is_system< T >;
		{
			t.startPass( info )
		};
		{
			t.wait()
		};
		{
			t.setupSystem( info )
		} -> std::same_as< vk::CommandBuffer& >;
	};

} // namespace fgl::engine
