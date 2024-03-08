//
// Created by kj16609 on 2/23/24.
//

#include "CullingSystem.hpp"

#include <tracy/TracyC.h>

#include "engine/FrameInfo.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/model/Model.hpp"

namespace fgl::engine
{

	void CullingSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Culling pass" );

		const auto frustum { info.camera_frustum };

		for ( auto* leaf : info.game_objects.getAllLeafsInFrustum( frustum ) )
		{
			assert( leaf );
			for ( auto& obj : *leaf )
			{
				//Has model?
				if ( obj.m_model )
				{
					//Test if the object is in the frustum

					const OrientedBoundingBox model_bounding_box {
						obj.m_model->getBoundingBox( Matrix< MatrixType::ModelToWorld >( obj.m_transform.mat4() ) )
					};

					obj.m_is_visible = frustum.intersects( model_bounding_box );

					//TODO: Drawing this fucking bounding box takes so much of the culling time. Literaly making it slower the more objects are in view.
					// I need to make this either to be async or to get fucked.
					if ( obj.m_is_visible )
					{
						//Draw the bounding box for debug
						//debug::world::drawBoundingBox( model_bounding_box );
					}
				}
			}
		}
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
