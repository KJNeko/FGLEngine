//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <concepts>

namespace fgl::engine::interface
{

	class NormalVector;

	//! Dummy class to allow for inheriting to pass 'is_bounding_box[
	struct BoundingBox
	{
	  public:

		constexpr static std::size_t POINT_COUNT { 8 };
		constexpr static std::size_t LINE_COUNT { ( POINT_COUNT / 2 ) * 3 };
		// Each point has 3 lines (Divide by 2 since we only need 1 line per direction)

		virtual ~BoundingBox() {};
	};

} // namespace fgl::engine::interface
