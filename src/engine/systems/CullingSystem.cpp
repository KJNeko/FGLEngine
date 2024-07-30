//
// Created by kj16609 on 2/23/24.
//

#include "CullingSystem.hpp"

#include <tracy/TracyC.h>

#include "engine/FrameInfo.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{
	static bool enable_culling { true };

	[[maybe_unused]] static bool& isCullingEnabled()
	{
		return enable_culling;
	}

	void CullingSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Culling pass" );

		const auto frustum { info.camera_data.camera->getFrustumBounds() };

		if ( !enable_culling )
		{
			return;
		}

		auto leafs { info.game_objects.getAllLeafsInFrustum( frustum ) };

		info.in_view_leafs = std::move( leafs );
	}

	void CullingSystem::runner()
	{
		TracyCSetThreadName( "Culling thread" );
		while ( !m_stop_token.stop_requested() )
		{
			m_start_sem.acquire();
			pass( *m_info.value() );
			m_end_sem.release();
		}
	}

	void CullingSystem::startPass( FrameInfo& info )
	{
		m_info = &info;
		m_start_sem.release();
	}

	void CullingSystem::wait()
	{
		m_end_sem.acquire();
	}

} // namespace fgl::engine
