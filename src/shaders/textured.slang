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
}

[ [ vk::binding( 0, 1 ) ] ]
ConstantBuffer< CameraData > camera : CAMERA;

[shader("vertex")]
CoarseVertex vertexMain( ModelVertex in_vertex )
{
	CoarseVertex out_vertex;

    vec4 world_pos = mul(in_vertex.model_matrix, vec4(in_vertex.simple.position, 1.0));

	const float4 transformed_pos = mul( mul( camera.projection, camera.view ), world_pos );
	out_vertex.position = transformed_pos;
	out_vertex.world_pos = world_pos.xyz;

	mat3 normal_matrix = transpose( inverse( mat3( in_vertex.model_matrix ) ) );

	out_vertex.normal = normalize( mul(normal_matrix, in_vertex.simple.normal) );
	out_vertex.tex_coord = in_vertex.simple.uv;
	out_vertex.material_id = in_vertex.material_id;

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
		if ( color.a == 0.0f )
			discard;
    }
	else
	{
		frag.color = material.color.factors;
  	}

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
	if ( normal.isTexture() )
	{
		const vec3 sample = texture( tex[ normal.texture_id ], vertex.tex_coord ).rgb;
		const vec3 scaled_sample = mul( sample, vec3(normal.scale) );
		frag.normal = vec4( scaled_sample, 1.0 );
    }
	else
	{
		frag.normal = vec4( vertex.normal, 1.0 );
	}

	return frag;
}
