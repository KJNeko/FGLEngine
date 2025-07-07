#version 450

module vertex;

import objects.gamemodel;

public struct SimpleVertex
{
	public vec3 position;
	public vec3 color;
};

public struct ModelVertex
{
	public SimpleVertex simple;
	public vec3 normal;
	public vec4 tangent;
	public vec2 uv;
	public InstanceRenderInfo instance;
};


