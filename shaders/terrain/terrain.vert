#version 450
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 instance_model_matrix;// 4, 5, 6, 7
layout (location = 8) in uint in_texture_id;

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec3 out_color;
layout (location = 2) out vec2 out_tex_coord;
layout (location = 3) out uint out_texture_idx;

void main() {

    gl_Position = instance_model_matrix * vec4(position, 1.0f);

    mat3 normal_matrix = transpose(inverse(mat3(instance_model_matrix)));

    out_normal = normalize(normal_matrix * normal);

    out_color = color;

    out_tex_coord = uv;

    out_texture_idx = in_texture_id;
}