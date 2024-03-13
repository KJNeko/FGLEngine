#version 450

layout(location = 0) in vec3 in_pos[];
layout(location = 1) in vec3 in_color[];
layout(location = 2) in vec3 in_normal[];
layout(location = 3) in vec2 in_uv[];

layout(location = 8) in uint in_tex_id[];

layout(location = 2) out vec3 out_normal[4];
layout(location = 3) out vec2 out_uv[4];

layout(vertices = 4) out;

void main()
{
    gl_TessLevelInner[0] = 2.0f;
    gl_TessLevelInner[1] = 2.0f;
    gl_TessLevelOuter[0] = 2.0f;
    gl_TessLevelOuter[1] = 2.0f;
    gl_TessLevelOuter[2] = 2.0f;
    gl_TessLevelOuter[3] = 2.0f;

    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
    out_normal[gl_InvocationID] = in_normal[gl_InvocationID];
    out_uv[gl_InvocationID] = in_uv[gl_InvocationID];
}