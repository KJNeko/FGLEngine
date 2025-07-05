#version 450

namespace vk
{

public struct DrawIndexedIndirectCommand
{
	public uint32_t index_count;
	public uint32_t instance_count;
	public uint32_t first_index;
	public int32_t vertex_offset;
	public uint32_t first_instance;
};

}