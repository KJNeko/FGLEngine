#version 450

layout(location = 0) in vec3 in_color;

#include "include/gbuffer_out.glsl"

void main() {
    out_color = vec4(in_color, 1.0f);
    out_normal = vec4(1.0f);
}