#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput i_position;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput i_normal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput i_albedo;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

/*
struct Light
{
    vec4 position;
    vec3 color;
    float radius;
};

layout (std410, binding = 3) buffer LightBuffer
{
    Light lights[];
};
*/

void main()
{
    vec3 position = subpassLoad(i_position).xyz;
    vec3 normal = subpassLoad(i_normal).xyz;
    vec3 albedo = subpassLoad(i_albedo).xyz;

    #define ambient 0.05

    vec3 frag_color = albedo.rbg * ambient;

    // Calculate sun light
    vec3 sun_dir = normalize(vec3(0.0, -0.5, 0.5));
    float sun_intensity = max(0.0, dot(normal, sun_dir));
    frag_color += sun_intensity * 0.5;

/*
    for (int i = 0; i < 4; i++)
    {
        vec3 L = lights[i].position.xyz - fragPos;
        float distance = length(L);
        L = normalize(L);
        float atten = lights[i].radius / (pow(dist, 3.0) + 1.0);

        vec3 N = normalize(normal);
        float NdotL = max(0.0, dot(N, L));
        vec3 diff = lights[i].color * albedo.rgb * NdotL * atten;

        frag_color += diff;
    }*/

    out_color = vec4(frag_color, 1.0);
}
