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

bool hasColorTexture()
{
    return materials[in_material_id].color_texture_id != INVALID_TEXTURE_ID;
}

bool hasMetallicTexture()
{
    return materials[in_material_id].metallic_texture_id != INVALID_TEXTURE_ID;
}

bool hasNormalTexture()
{
    return materials[in_material_id].normal_texture_id != INVALID_TEXTURE_ID;
}

bool hasOcclusionTexture()
{
    return materials[in_material_id].occlusion_texture_id != INVALID_TEXTURE_ID;
}

bool hasEmissiveTexture()
{
    return materials[in_material_id].emissive_texture_id != INVALID_TEXTURE_ID;
}

bool isValidTex(const uint id)
{
    return id != INVALID_TEXTURE_ID;
}
