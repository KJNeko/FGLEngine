#version 450

struct Vertex {
	vec2 uv;
	vec4 position : SV_Position;
};

[shader("vertex")]
Vertex vertexMain( uint index : SV_VertexID )
{
	Vertex vertex;
	vertex.uv = vec2( ( index << 1 ) & 2, index & 2 );
	vertex.position = vec4( vertex.uv * 2.0 - 1.0f, 0.0f, 0.0f );

	return vertex;
}

struct FragOut {
	vec4 color;
}

[shader("fragment")]
FragOut fragmentMain( Vertex vertex )
{
	FragOut frag;
    
	frag.color = vec4( 0.0f, 0.0f, 0.0f, 0.0f );

	return frag;
}









