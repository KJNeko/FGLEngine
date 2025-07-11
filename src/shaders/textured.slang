#version 450

import model.vertex;
import objects.camera;
import objects.gbuffer;
import material;

struct CoarseVertex {
	vec3 normal;
	vec2 tex_coord;
	float4 position : SV_Position;
	float3 world_pos;
	uint material_id;
	mat4x4 matrix;
	vec4 tangent;
};

[ [ vk::binding( 0, 1 ) ] ]
ConstantBuffer< CameraData > camera : CAMERA;

[shader("vertex")]
CoarseVertex vertexMain( ModelVertex in_vertex )
{
	CoarseVertex out_vertex;

    vec4 world_pos = mul(in_vertex.instance.model_matrix, vec4(in_vertex.simple.position, 1.0));

	const float4 transformed_pos = mul( camera.mat(), world_pos );
	out_vertex.position = transformed_pos;
	out_vertex.world_pos = world_pos.xyz;

	mat3 normal_matrix = transpose( inverse( mat3( in_vertex.instance.model_matrix ) ) );

	out_vertex.normal = normalize( mul(normal_matrix, in_vertex.normal) );
	out_vertex.tex_coord = in_vertex.uv;
	out_vertex.material_id = in_vertex.instance.material_id;
	out_vertex.matrix = in_vertex.instance.model_matrix;
	out_vertex.tangent = in_vertex.tangent;

	return out_vertex;
}

[[vk::binding(0,3)]]
ConstantBuffer<Material> materials[] : MATERIALS;

[[vk::binding(0,2)]]
Sampler2D[ ] tex : TEXTURES;

static const float MIN_ROUGHNESS = 0.04;

[shader("fragment")]
GBufferFragment fragmentMain( CoarseVertex vertex )
{
	GBufferFragment frag;
	frag.position = vertex.world_pos;

	vec3 diffuse_color;
	vec4 base_color;

	vec3 f0 = vec3( 0.04 );

	frag.color = vec4(1.0, 1.0, 1.0, 1.0);

	if ( vertex.material_id == INVALID_TEXTURE_ID )
	{
		frag.color = vec4( 227.0 / 255.0, 61. / 255.0, 148.0 / 255.0, 1.0 );
		return frag;
    }

    Material material = materials[vertex.material_id];

	frag.color = material.color.factors;

	if ( material.color.isTexture() )
	{
		vec4 color = texture( tex[ material.color.texture_id ], vertex.tex_coord );
		vec4 factors = material.color.factors;
		frag.color = color * factors;
    }
	else
	{
		frag.color = material.color.factors;
  	}

	if ( frag.color.w <= 0.0f ) discard;

	float metallic_scalar = 0.0;
	float roughness_scalar = 0.0;

	var metallic = material.metallic;
	if ( metallic.isTexture() )
	{
		vec4 sample = texture( tex[ metallic.texture_id ], vertex.tex_coord );

		metallic_scalar = sample.b * metallic.metallic_factor;
		roughness_scalar = sample.g * metallic.roughness_factor;
	}
	else
	{
		metallic_scalar = clamp( metallic.roughness_factor, MIN_ROUGHNESS, 1.0 );
		roughness_scalar = clamp( metallic.roughness_factor, 0.0, 1.0 );
    }

	float occlusion_scalar = 0.0;

	frag.metallic = vec3( metallic_scalar, roughness_scalar, occlusion_scalar );

	var normal = material.normal;
	// if ( normal.isTexture() && false )
	if ( normal.isTexture() )
	{
		// TANGENT SPACE WOOOOOOOOOOOOOOOOO
		// W is for some reason used to determine the signed direction of the tangent, Not sure why they do this
		// Perhaps look into it and figure out if we can save a packed byte?
		frag.normal = vec4( vertex.tangent.xyz * vertex.tangent.w, 1.0 );

		const vec3 N = vertex.normal;
		const vec3 T = vertex.tangent.xyz * vertex.tangent.w;
		const vec3 B = cross( N, T );
		const mat3 TBN = mat3( T, B, N );

		// Sample is in tangent space
		const vec3 sample_N = normalize( ( 2.0 * texture( tex[ normal.texture_id ], vertex.tex_coord ).rgb ) - 1.0 );
		const vec3 sample_NW = normalize( TBN * sample_N );
		const vec3 scaled_sample = normalize( sample_NW * normal.scale );
		frag.normal = vec4( scaled_sample, 1.0 );
    }
	else
	{
		frag.normal = vec4( vertex.normal, 1.0 );
	}

	return frag;
}
