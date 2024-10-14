layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 uv;

layout (location = 4) in mat4 instance_model_matrix;// 4, 5, 6, 7
layout (location = 8) in uint in_material_id;