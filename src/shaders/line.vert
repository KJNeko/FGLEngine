#version 450

layout(location = 0) in vec3 in_pos;

#include "include/camera.glsl"

void main()
{
    gl_Position = ubo.projection * ubo.view * vec4(in_pos, 1.0);
}
