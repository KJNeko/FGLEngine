#version 450

import vk.drawindexedindirect;
import bounds.axisalignedbb;
import objects.frustum;
import objects.gamemodel;


// in(vf)
ConstantBuffer< AxisAlignedBoundingBox > bounding_boxes[] : AA_BOUNDS;

// in constant (push constant maybe?:tm:)
ConstantBuffer< Frustum > frustum : FRUSTUM;

// in(c)
[[vk::binding(0,0)]]
StructuredBuffer< PrimitiveRenderInfo > primitives : PRIMITIVES;

// in(vr)
[[vk::binding(0,1)]]
StructuredBuffer< PrimitiveInstanceInfo > primitive_instances : PRIMITIVE_INSTANCES;

[[vk::binding(1,1)]]
StructuredBuffer< ModelInstanceInfo > model_instances : MODEL_INSTANCES;

// out
[[vk::binding(0,2)]]
RWStructuredBuffer< vk::DrawIndexedIndirectCommand > commands : COMMANDS;

// vertex info generated from compute shader. This information is used in the rendering process
[[vk::binding(1,2)]]
RWStructuredBuffer< InstanceRenderInfo > out_instances : OUT_INSTANCES;

//TODO: shader command constants
struct PushConstants
{
	// number of total models and their instances
	uint32_t draw_count;
};

[[push_constant]]
PushConstants pc;

[[shader("compute")]]
[numthreads(64,1,1)]
void computeMain( uint3 dispatch_id : SV_DispatchThreadID)
{
	// this will be dispatched with 0..N instances, each thread will be 1 instance from `model_instances`
	// each instance of this represents a unique primitive to be rendered
	const uint instance_index = dispatch_id.x; // global thread

	if (pc.draw_count == 0 || instance_index + 1 > pc.draw_count ) return;

	const PrimitiveInstanceInfo instance = primitive_instances[ instance_index ];
	const PrimitiveRenderInfo primitive = primitives[ instance.render_info_id ];

	//TODO: Cull (For now we will just pretend it's all in view)

	const bool in_view = true;

	if ( in_view )
	{
		// We instead will use the simpler approach of having a unique draw command for each instance of the model. in the future we might have a seperate processing segment for high-count items.
		vk::DrawIndexedIndirectCommand command;
		command.first_index = primitive.first_index;
		command.index_count = primitive.index_count;

		command.first_instance = instance_index;
		command.instance_count = 1;

		command.vertex_offset = primitive.first_vertex;

		commands[ instance_index ] = command;

		out_instances[ instance_index ].material_id = instance.material_id;

		const ModelInstanceInfo model_instance = model_instances[ instance.model_index ];
		out_instances[ instance_index ].model_matrix = model_instance.model_matrix;
		out_instances[ instance_index ].normal_matrix = model_instance.normal_matrix;
	}
	else
	{
		const vk::DrawIndexedIndirectCommand default_command = vk::DrawIndexedIndirectCommand( 0, 0, 0, 0, 0 );
		commands[ instance_index ] = default_command;
	}

}




