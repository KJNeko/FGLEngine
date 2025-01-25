//
// Created by kj16609 on 1/24/25.
//
#pragma once
#include <cstdint>
#include <vector>

#include "glm/vec3.hpp"

namespace fgl::engine
{
	using BVHIndex = std::uint32_t;

	class BVHTree
	{
		struct GameObjectInfo
		{};

		struct BVHNode
		{
			using FlagType = std::uint8_t;

			enum Masks : FlagType
			{
				Flag_None = 0,
				//! This node was visible during the last test
				Flag_Previously_Visible = 1 << 0,
				//! This node was not visible during the last test
				Flag_Previously_Invisible = 1 << 1,
			};

			FlagType m_flags { Masks::Flag_None };
			glm::vec3 m_centerpoint;

			//! Index to try next if we hit this index.
			BVHIndex m_hit;
			//! Index t otry if we fail this index.
			BVHIndex m_miss;
		};

	  public:
	};
} // namespace fgl::engine