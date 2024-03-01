//
// Created by kj16609 on 2/29/24.
//

#pragma once

#include <concepts>

namespace fgl::engine::interface
{

	class NormalVector;

	//! Dummy class to allow for inheriting to pass 'is_bounding_box[
	class BoundingBox
	{
	  public:

		virtual ~BoundingBox() {};
	};

} // namespace fgl::engine::interface
