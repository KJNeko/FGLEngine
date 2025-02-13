#version 450

import camera;
import gbuffer;

struct LineVertex
{
	float3 position : POSITION;
	float3 color : COLOR;
};

struct VertexOut
{
	float4 position : SV_Position;
	float3 color;
};

[ [ vk::binding( 0, 1 ) ] ]
ParameterBlock< CameraData > camera : CAMERA;

[shader("vertex")]
VertexOut vertexMain( LineVertex vertex )
{
	VertexOut out;

	out.position = camera.projection * camera.view * vec4( vertex.position, 1.0 );
	out.color = vertex.color;

	return out;
}

[shader("fragment")]
GBufferFragment fragmentMain( VertexOut vertex )
{
	GBufferFragment fragment;
	fragment.color = vec4( vertex.color, 1.0f );
	fragment.normal = vec4( 1.0f, 1.0f, 1.0f, 1.0f );
	fragment.position = vertex.position.xyz;
	fragment.metallic = vec3( 0.0f, 0.0f, 0.0f );

	return fragment;
}