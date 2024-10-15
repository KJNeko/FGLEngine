#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput i_color;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput i_position;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput i_normal;
layout (input_attachment_index = 3, binding = 3) uniform subpassInput i_metallic;
layout (input_attachment_index = 4, binding = 4) uniform subpassInput i_emissive;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main()
{
    vec3 position = subpassLoad(i_position).xyz;
    vec3 normal = subpassLoad(i_normal).xyz;
    vec3 color = subpassLoad(i_color).xyz;

    #define ambient 0.5

    const vec3 sun_dir = vec3(0.0, 0.0, -1.0);
    const vec3 sun_up = -sun_dir;

    const float diff = dot(sun_up, normal);

    // Calculate sun light
    vec3 sun_color = vec3(1.0);
    //vec3 sun_dir = normalize(vec3(0.0, -0.5, 0.5) - position);
    //float diff = max(dot(normalize(normal), sun_dir), 0.0);
    vec3 diffuse = diff * sun_color;

    vec3 frag_color = (ambient + diffuse) * color;

    out_color = vec4(frag_color, 1.0);
}
