//
// Created by kj16609 on 4/16/25.
//

#include "globals.hpp"

#include "constants.hpp"

namespace fgl::engine::global
{
	inline static FrameIndex CURRENT_FRAME_INDEX { 0 };

	FrameIndex getCurrentFrameIndex()
	{
		return CURRENT_FRAME_INDEX;
	}

	void indexNextFrame()
	{
		CURRENT_FRAME_INDEX = static_cast<
			FrameIndex >( ( CURRENT_FRAME_INDEX + static_cast< FrameIndex >( 1 ) ) % constants::MAX_FRAMES_IN_FLIGHT );
	}

	void setCurrentFrameIndex( const FrameIndex index )
	{
		CURRENT_FRAME_INDEX = index;
	}

} // namespace fgl::engine::global