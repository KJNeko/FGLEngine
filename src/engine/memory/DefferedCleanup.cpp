//
// Created by kj16609 on 4/16/25.
//

#include "DefferedCleanup.hpp"

#include "FGL_DEFINES.hpp"

namespace fgl::engine::memory
{

	DefferedCleanup::DefferedCleanup()
	{
		FGL_ASSERT( m_instance == nullptr, "Only one instance of DefferedCleanup should be created" );
		m_instance = this;
	}

	void DefferedCleanup::cleanIdx( const FrameIndex frame_index )
	{
		m_to_cleanup[ frame_index ].clear();
	}

} // namespace fgl::engine::memory
