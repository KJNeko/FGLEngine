//
// Created by kj16609 on 1/2/24.
//

#pragma once

namespace fgl::engine
{

	template < typename T >
	concept is_image = requires( T a ) {
		{
			a.getVkImage()
		} -> std::same_as< vk::Image& >;
		{
			a.format()
		} -> std::same_as< vk::Format >;
		{
			a.extent()
		} -> std::same_as< vk::Extent2D >;
	};

}