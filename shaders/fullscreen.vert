#version 450

// This vertex shader is meant to draw a single triangle on the entire screen butter so we can do full-screen effects.
// There *might* be a better way to do this. But it works for now.

layout (location = 0) out vec2 out_uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    out_uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(out_uv * 2.0f - 1.0f, 0.0f, 1.0f);
}