#version 450


layout (location = 0) in vec3 in_normal;
layout (location = 1) in vec2 in_tex_coord;

layout (location = 2) in flat uint material_id;

layout(set = 2, binding = 0) uniform sampler2D tex[];

//TODO: Sort this to be nice and not padded to shit
struct Material {
    uint color_tex_id;
    vec4 color_factors;

    uint metallic_roughness_tex_id;
    float metallic_factor;
    float roughness_factor;

    uint normal_tex_id;
    float normal_scale;

    uint occlusion_tex_id;
    float occlusion_strength;

    uint emissive_tex_id;
    vec3 emissive_factors;
};

layout(set = 3, binding = 0) uniform Material[] materials;

void main() {
    out_position = vec4(in_world_pos, 1.0f);

    vec3 N = vec3(0.0f);

    const uint in_normal_idx = materials[material_id].normal_tex_id;
    const uint in_albedo_idx = materials[material_id].color_tex_id;

    if (in_normal_idx == INVALID_TEXTURE_ID)
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
