#version 450

layout (location = 0) in vec2 frag_offset;
layout (location = 0) out vec4 out_color;

struct PointLight
{
    vec4 position;
    vec4 color;
};

layout (set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    vec4 ambient_light_color; // w is intensity
} ubo;

layout (set = 0, binding = 1) uniform LightingUBO
{
    PointLight lights[10];
    int num;
} point_lights;

layout (push_constant) uniform Push
{
    vec4 position;
    vec4 color;
    float radius;
} push;

const float M_PI = 3.14151926538;

void main() {
    float dis = sqrt(dot(frag_offset, frag_offset));

    if (dis >= 1.0)
    {
        discard;
    }

    out_color = vec4(push.color.xyz, 0.5 * (cos(dis * M_PI) + 1.0));
}