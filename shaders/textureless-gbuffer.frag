#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_world_pos;
layout (location = 3) in vec3 in_color;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_albedo;

layout (set = 0, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

#define NEAR_PLANE 0.01f
#define FAR_PLANE 1000.0f

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main()
{
    out_position = vec4(in_world_pos, 1.0f);

    vec3 N = normalize(in_normal);

    out_normal = vec4(N, 1.0);

    out_position.a = linearDepth(out_position.z);

    out_albedo = vec4(in_color, 1.0);
}