#version 450
#extension GL_EXT_debug_printf: enable
#extension GL_GOOGLE_include_directive : enable

#include "include/vertex.glsl"

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out vec3 out_world_pos;
layout (location = 3) out flat uint out_albedo_id;
layout (location = 4) out flat uint out_normal_id;
layout (location = 5) out flat uint out_metallic_roughness_id;

#include "include/camera.glsl"

void main() {

    vec4 position_world = instance_model_matrix * vec4(position, 1.0);

    gl_Position = ubo.projection * ubo.view * position_world;
    out_world_pos = vec3(gl_Position);

    mat3 normal_matrix = transpose(inverse(mat3(instance_model_matrix)));

    out_normal = normalize(normal_matrix * normal);

    out_tex_coord = uv;

    out_albedo_id = in_albedo_id;
    out_metallic_roughness_id = in_metallic_roughness_id;
}