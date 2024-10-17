#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput i_color;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput i_position;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput i_normal;
layout (input_attachment_index = 3, binding = 3) uniform subpassInput i_metallic;
//layout (input_attachment_index = 4, binding = 4) uniform subpassInput i_specular;
layout (input_attachment_index = 4, binding = 4) uniform subpassInput i_emissive;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

layout (set = 1, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} camera_info;

//TODO: uniform binding with sun information

vec3 getCameraPosition()
{
    return vec3(
    camera_info.inverse_view[3][0],
    camera_info.inverse_view[3][1],
    camera_info.inverse_view[3][2]
    );
}

vec3 sun_dir = vec3(-1.0, -1.0, -1.0);

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
    float alpha   = roughness * roughness;
    float alphaSq = alpha * alpha;

    float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
    return alphaSq / (PI * denom * denom);
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

vec3 schlick(vec3 F0, float cosTheta)
{
    return F0 + (vec3(1.0) - F0) * pow(1.0 - cosTheta, 5.0);
}

void main()
{
    const vec3 albedo = subpassLoad(i_color).xyz;

    const vec3 position = subpassLoad(i_position).xyz;
    const vec3 normal = subpassLoad(i_normal).xyz;

    // metallic, roughness, occlusion
    // r,        g,         b
    const vec3 metallic_comb = subpassLoad(i_metallic).xyz;

    // split the channels of ease of use
    const float metallic_value = metallic_comb.r;
    const float roughness_value = metallic_comb.g;
    const float occlusion = metallic_comb.b;

    const vec3 camera_pos = getCameraPosition();

    // Calculate the vector from the world position to the camera
    const vec3 Lo = normalize(camera_pos - position);
    const vec3 N = normalize(normal);

    float cosLo = max(dot(N, Lo), 0.0);

    // Specular reflection
    //vec3 Lr = 2.0 * cosLo * N - Lo;
    vec3 Lr = reflect(-Lo, N);

    const vec3 fresnel_factor = vec3(0.04);

    // Fresnel
    vec3 F0 = mix(fresnel_factor, albedo, metallic_value);

    vec3 direct_lighting = vec3(0.0);

    // Do this for each light
    {
        vec3 Li = -sun_dir;
        vec3 Lradiance = vec3(1.0);// color?

        // half vector
        vec3 Lh = normalize(Li + Lo);

        float cosLi = max(dot(N, Li), 0.0);
        float cosLh = max(dot(N, Lh), 0.0);

        vec3 F = schlick(F0, max(dot(Lh, Lo), 0.0));
        float D = ndfGGX(cosLh, roughness_value);
        float G = gaSchlickGGX(cosLi, cosLo, roughness_value);

        vec3 kb = mix(vec3(1.0) - F, vec3(0.0), metallic_value);
        vec3 diffuse_BRDF = kb * albedo;
        vec3 specular_BRDF = (F * D * G) / max(0.04, 4.0 * cosLi * cosLo);

        direct_lighting = (diffuse_BRDF + specular_BRDF) * Lradiance * cosLi;
    }

    vec3 ambient_lighting = albedo * 0.1;

    out_color = vec4(direct_lighting + ambient_lighting, 1.0);
}
