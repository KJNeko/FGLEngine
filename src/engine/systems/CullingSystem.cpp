//
// Created by kj16609 on 2/23/24.
//

#include "CullingSystem.hpp"

#include "engine/debug/drawers.hpp"
#include "engine/model/BoundingBox.hpp"
#include "engine/model/Model.hpp"

namespace fgl::engine
{

	void CullingSystem::pass( FrameInfo& info )
	{
		ZoneScoped;

		const auto frustum { info.camera_frustum };

		for ( auto& [ id, obj ] : info.game_objects )
		{
			//Has model?
			if ( obj.model )
			{
				//Test if the object is in the frustum

				const BoundingBox model_bounding_box {
					obj.model->getBoundingBox( Matrix< MatrixType::ModelToWorld >( obj.transform.mat4() ) )
				};

				obj.is_visible = model_bounding_box.isInFrustum( frustum );

				if ( obj.is_visible )
				{
					//Draw the bounding box for debug
					debug::world::drawBoundingBox( model_bounding_box, info.camera );
				}
			}
		}
	}

} // namespace fgl::engine
