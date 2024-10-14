#version 450
#extension GL_EXT_nonuniform_qualifier: enable
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_uv;
layout (location = 2) in vec3 in_world_pos;
layout (location = 3) in flat uint in_material_id;

#include "include/gbuffer_out.glsl"

layout (set = 1, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} camera_info;

layout (set = 2, binding = 0) uniform sampler2D tex[];

#include "include/material.glsl"

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

    uint normal_id = materials[in_material_id].normal_texture_id;
    if (isValidTex(normal_id))
    {
        vec3 normal_vec = texture(tex[normal_id], in_uv).xyz;
        out_normal = vec4(normal_vec, 1.0);
    }
    else
    {
        out_normal = vec4(in_normal, 1.0f);
    }

    uint albedo_id = materials[in_material_id].color_texture_id;

    if (albedo_id == INVALID_TEXTURE_ID)
    {
        out_albedo = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        vec4 tex_value = texture(tex[albedo_id], in_uv);

        if (tex_value.a < 1.0)
        {
            discard;
        }

        out_albedo = tex_value;
    }

    out_position.a = linearDepth(out_position.z);
}