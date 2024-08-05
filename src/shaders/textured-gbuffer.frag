#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_world_pos;
layout (location = 3) in flat uint in_albedo_idx;
layout (location = 4) in flat uint in_normal_idx;
layout (location = 5) in flat uint in_metallic_roughness_idx;

layout (location = 0) out vec4 out_position;
layout (location = 1) out vec4 out_normal;
layout (location = 2) out vec4 out_albedo;

layout (set = 1, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} camera_info;

layout (set = 2, binding = 0) uniform sampler2D tex[];

#define NEAR_PLANE 0.01f
#define FAR_PLANE 1000.0f

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

uint INVALID_TEX_ID = 0;

void main()
{
    out_position = vec4(in_world_pos, 1.0f);


    vec3 N = vec3(0.0f);

    if (in_normal_idx == INVALID_TEX_ID)
    {
        N = normalize(in_normal);
    }
    else
    {
        N = texture(tex[in_normal_idx], in_tex_coord).xyz;
    }

    out_normal = vec4(N, 1.0);

    vec4 tex_value = texture(tex[in_albedo_idx], in_tex_coord);

    if (tex_value.a < 1.0)
    {
        discard;
    }

    out_albedo = tex_value;

    out_position.a = linearDepth(out_position.z);
}