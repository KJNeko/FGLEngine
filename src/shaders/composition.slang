#version 450

import objects.camera;

struct Vertex {
	vec4 position : SV_Position;
};

[shader("vertex")]
Vertex vertexMain( uint index : SV_VertexID )
{
	Vertex vertex;
	vec2 uv = vec2( ( index << 1 ) & 2, index & 2 );

	vec2 mulled = uv * vec2( 2.0 );
	vec2 minone = mulled - vec2( 1.0, 1.0 );

	vertex.position = vec4(minone, 0.0f, 1.0f );

	return vertex;
}

struct CompositeFragment {
	vec4 color;
}

struct GBufferInput {
	SubpassInput<vec4> color : COLOR;
	SubpassInput<vec4> position : POSITION;
	SubpassInput<vec4> normal : NORMAL;
	SubpassInput<vec4> metallic : METALLIC;
	SubpassInput<vec4> emissive : EMISSIVE;
};

// SubpassInput< vec4 > test : TEST;

[[vk::binding(0,0)]]
ParameterBlock<GBufferInput> gbuffer : GBUFFER;

// SubpassInput<vec4> color : COLOR;
// SubpassInput<vec4> position : POSITION;
// SubpassInput<vec4> normal : NORMAL;
// SubpassInput<vec4> metallic : METALLIC;
// SubpassInput<vec4> emissive : EMISSIVE;

[[vk::binding(0,1)]]
ParameterBlock< CameraData > camera : CAMERA;

//TODO: constant flags

vec3 sun_dir = normalize(vec3(-1.0, -1.0, -1.0));

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (3.145 * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
    return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;// Epic suggests using this roughness remapping for analytic lights.
    return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

//TODO: Apparently this can be gotten from a texture instead?
vec3 schlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


[shader("fragment")]
CompositeFragment fragmentMain( Vertex vertex )
{
	// const vec3 test_sample = test.SubpassLoad().xyz;
	// const vec3 albedo = gbuffer.color.SubpassLoad().xyz * test_sample;
	const vec3 albedo = gbuffer.color.SubpassLoad().xyz;

	const vec3 position = gbuffer.position.SubpassLoad().xyz;
	const vec3 normal = gbuffer.normal.SubpassLoad().xyz;

    // metallic, roughness, occlusion
    // r,        g,         b
	const vec3 metallic_comb = gbuffer.metallic.SubpassLoad().xyz;

	// frag.color = vec4(albedo, 1.0f);

	const float metallic = metallic_comb.r;
	const float roughness = metallic_comb.g;
	const float occlusion = metallic_comb.b;

	const vec3 camera_pos = camera.getPos();

	const vec3 Lo = normalize( camera_pos - position );
	const vec3 N = normalize( normal );

    float cosLo = max(dot(N, Lo), 0.0);

	vec3 Lr = reflect( -Lo, N );

	const vec3 fresnel_factor = vec3( 0.04 );

	vec3 F0 = mix( fresnel_factor, albedo, metallic );

	vec3 direct_lighting = vec3( 0.0 );

	vec3 Li = -sun_dir;
    vec3 Lradiance = vec3(1.0);

	vec3 Lh = normalize( Li + Lo );

	float t = dot(normal, sun_dir);

	// frag.color = vec4( vec3( t ), 1.0 );
	// frag.color = vec4(normal, 1.0);

	float cosLi = max( dot( N, Li ), 0.0 );
	float cosLh = max( dot( N, Lh ), 0.0 );

	float cosTheta = dot( Li, N );

	const vec3 F = schlick( F0, cosTheta );
	const float D = ndfGGX( cosLh, roughness );
	const float G = gaSchlickGGX( cosLi, cosLo, roughness );

	vec3 kb = mix( vec3( 1.0 ) - F, vec3( 0.0 ), metallic );
	vec3 diffuse_BRDF = kb * albedo;
	vec3 specular_BRDF = ( F * D * G ) / max( 0.04, 4.0 * cosLi * cosLo );

	direct_lighting = ( diffuse_BRDF + specular_BRDF ) * Lradiance * cosLi;

	vec3 ambient_lighting = albedo * 0.1;

	frag.color = vec4( direct_lighting + ambient_lighting, 1.0 );

	return frag;
}