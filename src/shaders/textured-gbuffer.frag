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

// Include all helper functions for glsl.
#include "include/pbr.glsl"

float linearDepth(float depth)
{
    float z = depth * 2.0f - 1.0f;
    return (2.0f * NEAR_PLANE * FAR_PLANE) / (FAR_PLANE + NEAR_PLANE - z * (FAR_PLANE - NEAR_PLANE));
}

vec3 error_color = vec3(0.475, 0.0, 0.686);

float MIN_ROUGHNESS = 0.04;

void main()
{
    out_position = in_world_pos;

    vec3 diffuse_color;
    vec4 base_color;

    vec3 f0 = vec3(0.04);

    uint mat_id = in_material_id;

    if (mat_id == INVALID_TEXTURE_ID)
    {
        discard;
    }

    // The Vulkan-glTF-PBR example does some alpha mask stuff. I'm not exactly sure what it's used for
    // So for now we will just set the base_color to be the texture color
    uint color_texture_id = materials[mat_id].color_texture_id;
    if (color_texture_id != INVALID_TEXTURE_ID)
    {
        // Load the texture and multiply it against the color factors
        vec4 color = texture(tex[color_texture_id], in_uv).rgba;
        vec4 factors = materials[mat_id].color_factors;
        out_color = color * factors;
    }
    else
    {
        // If we have no color texture, Simply use the color factors as our color
        out_color = materials[mat_id].color_factors;
    }

    float metallic_scalar = 0.0;
    float roughness_scalar = 0.0;

    uint metallic_id = materials[mat_id].metallic_texture_id;
    if (metallic_id != INVALID_TEXTURE_ID)// If the metallic texture is assigned, Then we should use it. Otherwise we want to use the metallic factor
    {
        vec4 tex_sample = texture(tex[metallic_id], in_uv).rgba;
        // r channel is for occlusion data (optional)
        // g channel is for roughnes factor
        // b channel is for metallic factor
        metallic_scalar = tex_sample.b * materials[mat_id].metallic_factor;
        roughness_scalar = tex_sample.g * materials[mat_id].roughness_factor;
    }
    else // Texture was not present, So we instead use the factor values as our values
    {
        metallic_scalar = clamp(materials[mat_id].metallic_factor, MIN_ROUGHNESS, 1.0);
        roughness_scalar = clamp(materials[mat_id].roughness_factor, 0.0, 1.0);
    }

    // The example does some magic here that I don't understand?
    // https://github.com/SaschaWillems/Vulkan-glTF-PBR/blob/master/data/shaders/material_pbr.frag#L242
    float occlusion_scalar = 0.0;

    out_metallic = vec3(metallic_scalar, roughness_scalar, occlusion_scalar);
}