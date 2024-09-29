#version 450

layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;

#include "include/camera.glsl"

layout(location = 0) out vec3 out_color;

void main()
{
    gl_Position = ubo.projection * ubo.view * vec4(in_pos, 1.0);
    out_color = in_color;
}
