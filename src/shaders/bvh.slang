#version 450

typedef uint32_t BVHIndex;
typedef uint8_t BVHFlags;

public struct BVHNode
{
	BVHFlags m_flags;
	float3 m_centerpoint;
	float3 m_scale;
	float4 m_rotation;

	BVHIndex next_hit;
    BVHIndex next_miss;
}




