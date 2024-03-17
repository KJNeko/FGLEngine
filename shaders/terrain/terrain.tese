#version 450
#extension GL_EXT_nonuniform_qualifier: enable

layout(quads, equal_spacing, cw) in;

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec2 in_uv[];
layout(location = 2) in flat uint in_tex_idx[];
layout(location = 3) in flat float in_scale_z[];

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec3 out_world_pos;
layout(location = 3) out uint out_tex_idx;

layout(set = 0, binding = 0) uniform CameraInfo
{
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

layout (set = 1, binding = 0) uniform sampler2D tex[];

void main()
{
    vec2 uv1 = mix(in_uv[0], in_uv[1], gl_TessCoord.x);
    vec2 uv2 = mix(in_uv[3], in_uv[2], gl_TessCoord.x);
    out_uv = mix(uv1, uv2, gl_TessCoord.y);

    vec3 n1 = mix(in_normal[0], in_normal[1], gl_TessCoord.x);
    vec3 n2 = mix(in_normal[3], in_normal[2], gl_TessCoord.x);
    out_normal = mix(n1, n2, gl_TessCoord.y);

    vec4 pos1 = mix(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_TessCoord.x);
    vec4 pos2 = mix(gl_in[3].gl_Position, gl_in[2].gl_Position, gl_TessCoord.x);
    vec4 pos = mix(pos1, pos2, gl_TessCoord.y);

    out_tex_idx = in_tex_idx[0];

    // Doesn't matter which in_scale_z we take since it'll be the same for the entire model.
    pos.z += (texture(tex[out_tex_idx], out_uv).r - 0.5) * in_scale_z[0];

    vec4 world_pos = ubo.projection * ubo.view * pos;

    out_world_pos = vec3(world_pos);
    gl_Position = world_pos;
}