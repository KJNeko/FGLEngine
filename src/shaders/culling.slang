#version 450

import vk.drawindexedindirect;
import bounds.axisalignedbb;
import objects.frustum;
import objects.gamemodel;



struct ModelInstance
{
	uint32_t model_index;
	mat4x4 matrix;
};

RWStructuredBuffer< vk::DrawIndexedIndirectCommand > commands : COMMANDS;
RWStructuredBuffer< ModelInstance > instances : COMMAND_INSTANCES;

ConstantBuffer< AxisAlignedBoundingBox > bounding_boxes[] : BOUNDS;

ConstantBuffer< Frustum > frustum : FRUSTUM;

ConstantBuffer< ModelInstance > model_instances[] : MODEL_INSTANCES;
ConstantBuffer< GameModel > models[] : MODELS;

//TODO: shader command constants
struct PushConstants
{
	// number of total models and their instances
	uint32_t draw_count;
	uint32_t start_idx;
	uint32_t end_idx;
};

ConstantBuffer<PushConstants> pc;


[[shader("compute")]]
[numthreads(64,1,1)]
void main( uint3 dispatch_id : SV_DispatchThreadID)
{
	// this will be dispatched with 0..N instances, each thread will be 1 instance from `model_instances`
	uint instance_index = pc.start_idx + dispatch_id.x; // global thread

	if ( dispatch_id.x > pc.end_idx ) return;
	if ( instance_index > pc.draw_count ) return;

	ModelInstance model_instance = model_instances[ instance_index ];
	var model_index = model_instance.model_index;

	GameModel model = models[ model_index ];

	//TODO: Cull (For now we will just pretend it's all in view)

	const bool in_view = true;

	if ( in_view )
	{
		uint32_t old_instance_count = 0;
		InterlockedAdd( commands[ model_index ].instance_count, 1, old_instance_count );

		var command = &commands[ model_index ];

		// if the old instance count was zero, we are the first command and need to populate.
		if ( old_instance_count == 0 )
		{
			// first index, populate the command's data

			//TODO: Populate from GameModel once i've actually implemented that class properly
			command->first_index = 0;
			command->index_count = 0;
			command->vertex_offset = 0;

			atomicAdd( command->first_instance, instance_index );
		}

		barrier();

		var idx = command->first_instance + old_instance_count;

		instances[ instance_index ] = model_instances[ idx ];
	}
}




