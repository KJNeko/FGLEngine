#version 450

module vertex;

public struct SimpleVertex
{
	public vec3 position;
	public vec3 color;
};

public struct ModelVertex
{
	public SimpleVertex simple;
	public vec3 normal;
	public vec3 tangent;
	public vec2 uv;
	public mat4x4 model_matrix;
	public mat4x4 normal_matrix;
	public uint material_id;
};


