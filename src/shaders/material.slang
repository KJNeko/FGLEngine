#version 450

module material;

public static const flat uint INVALID_TEXTURE_ID = 0;

public struct Albedo {
	public flat uint texture_id;
	public vec4 factors;

	public bool isTexture()
    {
        return texture_id != INVALID_TEXTURE_ID;
    }
};

public struct Metallic
{
    public flat uint texture_id;
	public float metallic_factor;
	public float roughness_factor;

	public bool isTexture()
    {
        return texture_id != INVALID_TEXTURE_ID;
    }
}

public struct Normal
{
	public flat uint texture_id;
	public float scale;

	public bool isTexture()
    {
        return texture_id != INVALID_TEXTURE_ID;
    }
}

public struct Occlusion
{
	public flat uint texture_id;
	public float strength;

	public bool isTexture()
    {
        return texture_id != INVALID_TEXTURE_ID;
    }
}

public struct Emissive
{
	public flat uint texture_id;
	public vec3 factors;

	public bool isTexture()
    {
        return texture_id != INVALID_TEXTURE_ID;
    }
}


public struct Material 
{
	public Albedo color;
	public Metallic metallic;
	public Normal normal;
	public Occlusion occlusion;
	public Emissive emissive;
}

