#version 450

module gbuffer;

public struct GBufferFragment
{
	public vec4 color : COLOR;
	public vec3 position : POSITION;
	public vec4 normal : NORMAL;
	public vec3 metallic : METALLIC;
	public vec3 emissive : EMISSIVE;
};

