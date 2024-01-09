#version 450

layout (location = 0) in vec3 frag_color;
layout (location = 1) in vec3 frag_pos_world;
layout (location = 2) in vec3 frag_normal_world;

layout (location = 0) out vec4 out_color;

layout (set = 0, binding = 0) uniform CameraInfo {
    mat4 projection;
    mat4 view;
    mat4 inverse_view;
} ubo;

struct PointLight
{
    vec4 position;
    vec4 color;
};

layout (set = 0, binding = 2) uniform lights
{
    int num_point_lights;
    PointLight lights[10];
} point_lights;

const vec4 ambient_light_color = vec4(0.5, 0.5, 0.5, 1.0); // w is intensity

void main()
{
    vec3 diffuse_light = ambient_light_color.xyz * ambient_light_color.w;
    vec3 specular_light = vec3(0.0);
    vec3 surface_normal = normalize(frag_normal_world);

    vec3 camera_pos_world = ubo.inverse_view[3].xyz;
    vec3 view_direction = normalize(camera_pos_world - frag_pos_world);

    for (int i = 0; i < point_lights.num_point_lights; ++i)
    {
        PointLight light = point_lights.lights[i];
        vec3 direction_to_light = light.position.xyz - frag_pos_world;
        float attenuation = 1.0 / dot(direction_to_light, direction_to_light); // distance squared
        direction_to_light = normalize(direction_to_light);

        float cos_ang_incidence = max(dot(surface_normal, direction_to_light), 0);
        vec3 intensity = light.color.xyz * light.color.w * attenuation;

        diffuse_light += intensity * cos_ang_incidence;

        //specular light
        vec3 half_angle = normalize(direction_to_light + view_direction);
        float blinnTerm = dot(surface_normal, half_angle);
        blinnTerm = clamp(blinnTerm, 0, 1);
        blinnTerm = pow(blinnTerm, 64.0); // higher -> shaper highlight
        specular_light += intensity * blinnTerm;
    }

    out_color = vec4(diffuse_light * frag_color + specular_light * frag_color, 1.0);
}