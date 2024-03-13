#version 450


layout(quads, equal_spacing, cw) in;

layout(location = 2) in vec3 in_normal[];
layout(location = 3) in vec2 in_uv[];

layout(location = 8) in uint in_tex_id[];

layout(location = 0) out vec3 out_normal;
layout(location = 3) out vec2 out_uv;

struct FrustumPlane
{
    vec3 pos;
    float dist;
};

layout(set = 0, binding = 0) uniform CameraInfo
{
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
    FrustumPlane frustum_planes[6];
} ubo;

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

    gl_Position = ubo.projection * ubo.view * pos;
}