#version 450

struct PrimitiveRenderInfo {
    //! Where in the buffer the first vertex lies
	uint32_t first_vertex;

    //! Where in the buffer the first index would be
    uint32_t first_index;

    //! Number of indicies there are
    uint32_t index_count;
};

struct PrimitiveInstanceInfo
{
	uint32_t render_info_id;
	uint32_t model_index;
	uint32_t material_id;
}

struct InstanceRenderInfo
{
	mat4x4 matrix;
	uint32_t mat_id;
}

struct ModelInstanceInfo {
	mat4x4 model_matrix;  
};
