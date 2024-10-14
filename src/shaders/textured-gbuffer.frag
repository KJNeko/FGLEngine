#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coord;
layout (location = 2) in vec3 in_world_pos;
layout (location = 3) in flat uint in_material_id;

#include "include/gbuffer_out.glsl"

layout (set = 1, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} camera_info;

layout (set = 2, binding = 0) uniform sampler2D tex[];

layout (set = 3, binding = 0) uniform Material {
    uint color_texture_id;
    vec4 color_factors;

    uint metallic_texture_id;
    float metallic_factor;
    float roughness_factor;

    uint normal_texture_id;
    float normal_scale;

    uint occlusion_texture_id;
    float occlusion_strength;

    uint emissive_texture_id;
    vec3 emissive_factors;
} materials[];

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

void main()
{
    out_position = vec4(in_world_pos, 1.0f);

    if (in_material_id == INVALID_TEXTURE_ID)
    {
        out_albedo = vec4(255.0f / 255.0f, 192.0f / 255.0f, 203.0f / 255.0f, 1.0f);
        return;
    }

    uint in_normal_idx = materials[in_material_id].normal_texture_id;
    vec3 N = vec3(0.0f);

    if (in_normal_idx == INVALID_TEXTURE_ID)
    {
        N = normalize(in_normal);
    }
    else
    {
        N = texture(tex[in_normal_idx], in_tex_coord).xyz;
    }

    out_normal = vec4(N, 1.0);

    uint in_albedo_idx = materials[in_material_id].color_texture_id;
    vec4 tex_value = texture(tex[in_albedo_idx], in_tex_coord);

    if (tex_value.a < 1.0)
    {
        discard;
    }

    out_albedo = tex_value;

    out_position.a = linearDepth(out_position.z);
}