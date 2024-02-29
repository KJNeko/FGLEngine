//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <concepts>

namespace fgl::engine::interface
{

	//! Dummy class to allow for inheriting to pass 'is_bounding_box[
	class OrientedBoundingBox
	{};

	template < typename T > concept is_bounding_box = std::is_base_of_v< OrientedBoundingBox, T >;

} // namespace fgl::engine::interface
