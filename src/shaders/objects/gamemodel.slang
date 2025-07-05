#version 450

public struct PrimitiveRenderInfo {
    //! Where in the buffer the first vertex lies
	public uint32_t first_vertex;

    //! Where in the buffer the first index would be
    public uint32_t first_index;

    //! Number of indicies there are
    public uint32_t index_count;
};

// Each primitive has one instance
public struct PrimitiveInstanceInfo
{
	public uint32_t render_info_id;
	public uint32_t model_index;
	public uint32_t material_id;
};

// One object exists for each render instance
public struct InstanceRenderInfo
{
	public mat4x4 model_matrix;
	public mat4x4 normal_matrix;
	public uint32_t material_id;
};

// One exists for each model (primitive collection)
public struct ModelInstanceInfo 
{
	public mat4x4 model_matrix;
	public mat4x4 normal_matrix;
};
