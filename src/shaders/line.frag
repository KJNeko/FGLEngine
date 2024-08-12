#version 450

#include "include/gbuffer_out.glsl"

void main() {
    out_albedo = vec4(1.0, (20.0 / 255.0), (147.0 / 255.0f), 1.0f);
    out_normal = vec4(1.0f);
}