#version 450


layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 instance_model_matrix;// 4, 5, 6, 7
layout (location = 8) in uint material_id;


layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coord;

layout (location = 2) out flat uint out_material_id;

void main() {

    vec4 pos_world = instance_model_matrix * vec4(position, 1.0);

    gl_Position = ubo.projection * ubo.view * pos_world;

    mat3 normal_matrix = transpose(inverse(mat3(instance_model_matrix)));

    out_normal = normalize(normal_matrix * normal);

    out_tex_coord = uv;

    out_material_id = material_id;
}
