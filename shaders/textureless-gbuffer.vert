#version 450
#extension GL_EXT_debug_printf: enable

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 instance_model_matrix;// 4, 5, 6, 7

layout (location = 0) out vec3 out_normal;
layout (location = 1) out vec2 out_tex_coord;
layout (location = 2) out vec3 out_world_pos;
layout (location = 3) out vec3 out_color;

layout (set = 0, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

void main() {

    vec4 position_world = instance_model_matrix * vec4(position, 1.0);

    gl_Position = ubo.projection * ubo.view * position_world;
    out_world_pos = vec3(gl_Position);

    mat3 normal_matrix = transpose(inverse(mat3(instance_model_matrix)));

    out_normal = normalize(normal_matrix * normal);

    out_tex_coord = uv;

    out_color = color;
}