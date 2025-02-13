#version 450

module vertex;

public struct SimpleVertex
{
	public vec3 position;
	public vec3 color;
	public vec3 normal;
	public vec2 uv;
};

public struct ModelVertex
{
	public SimpleVertex simple;
	public mat4x4 model_matrix;
	public uint material_id;
};


