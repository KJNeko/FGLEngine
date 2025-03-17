#version 450

namespace vk
{

struct DrawIndexedIndirectCommand
{
	uint32_t index_count;
	uint32_t instance_count;
	uint32_t first_index;
	int32_t vertex_offset;
	uint32_t first_instance;
};

}