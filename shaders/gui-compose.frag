#version 450

layout (input_attachment_index = 0, binding = 0) uniform subpassInput i_composite;

layout (location = 0) in vec2 in_uv;

layout (location = 0) out vec4 out_color;

void main() {
    vec4 composite = subpassLoad(i_composite).xyzw;

    out_color = composite;
}


