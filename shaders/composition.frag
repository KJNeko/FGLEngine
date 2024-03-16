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

    // Calculate sun light
    vec3 sun_color = vec3(0.2);
    vec3 sun_dir = normalize(vec3(0.0, -0.5, 0.5) - position);
    float diff = max(dot(normalize(normal), sun_dir), 0.0);
    vec3 diffuse = diff * sun_color;

    vec3 frag_color = (ambient + diffuse) * albedo;

    out_color = vec4(frag_color, 1.0);
}
