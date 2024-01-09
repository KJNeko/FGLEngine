#version 450

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 instance_model_matrix;
layout (location = 8) in mat4 instance_normal_matrix;

layout (location = 0) out vec3 frag_color;
layout (location = 1) out vec3 frag_pos_world;
layout (location = 2) out vec3 frag_normal_world;

layout (set = 0, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

struct PointLight
{
    vec4 position;
    vec4 color;
};

layout (set = 0, binding = 2) uniform lights
{
    int num_point_lights;
    PointLight light[10];
} point_lights;

void main() {
    vec4 position_world = instance_model_matrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * position_world;

    frag_normal_world = normalize(mat3(instance_normal_matrix) * normal);
    frag_pos_world = position_world.xyz;
    frag_color = color;
}