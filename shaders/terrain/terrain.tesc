#version 450

layout(location = 0) in vec3 in_normal[];
layout(location = 1) in vec3 in_color[];
layout(location = 2) in vec2 in_uv[];
layout(location = 3) in flat uint in_tex_idx[];
layout(location = 4) in flat float in_scale_z[];

layout(vertices = 4) out;

layout(location = 0) out vec3 out_normal[4];
layout(location = 1) out vec2 out_uv[4];
layout(location = 2) out flat uint out_tex_idx[4];
layout(location = 3) out flat float out_scale_z[4];

layout (set = 0, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

vec3 midpoint(vec3 point1, vec3 point2)
{
    return (point1 + point2) / vec3(2, 2, 2);
}

float tesselationLevel(vec4 point1, vec4 point2)
{
    vec3 mid = midpoint(vec3(point1), vec3(point2));

    vec3 camera_pos = ubo.inverse_view[3].xyz;

    float max_tess = 32.0f;
    float dist_multip = 8.0f;

    float dist = max_tess - (distance(camera_pos, mid) / dist_multip);

    return clamp(dist, 1.0f, max_tess);
}

void main()
{
    gl_TessLevelOuter[0] = tesselationLevel(gl_in[3].gl_Position, gl_in[0].gl_Position);
    gl_TessLevelOuter[1] = tesselationLevel(gl_in[0].gl_Position, gl_in[1].gl_Position);
    gl_TessLevelOuter[2] = tesselationLevel(gl_in[1].gl_Position, gl_in[2].gl_Position);
    gl_TessLevelOuter[3] = tesselationLevel(gl_in[2].gl_Position, gl_in[3].gl_Position);

    gl_TessLevelInner[0] = mix(gl_TessLevelOuter[1], gl_TessLevelOuter[3], 0.5);
    gl_TessLevelInner[1] = mix(gl_TessLevelOuter[0], gl_TessLevelOuter[2], 0.5);

    gl_out[gl_InvocationID].gl_Position =  gl_in[gl_InvocationID].gl_Position;
    out_normal[gl_InvocationID] = in_normal[gl_InvocationID];
    out_uv[gl_InvocationID] = in_uv[gl_InvocationID];
    out_tex_idx[gl_InvocationID] = in_tex_idx[gl_InvocationID];
    out_scale_z[gl_InvocationID] = in_scale_z[gl_InvocationID];
}