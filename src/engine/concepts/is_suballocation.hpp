//
// Created by kj16609 on 12/29/23.
//

#pragma once

namespace fgl::engine::memory
{

	template < typename T >
	concept is_suballocation = requires( const T t ) {
		{ t->getVkBuffer() } -> std::same_as< vk::Buffer >;
		{ t->getOffset() } -> std::same_as< vk::DeviceSize >;
		/*
		{
			t.size()
		} -> std::same_as< vk::DeviceSize >;
		*/
		{ t->getDescriptorInfo() } -> std::same_as< vk::DescriptorBufferInfo >;
	};
} // namespace fgl::engine::memory