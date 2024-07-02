//
// Created by kj16609 on 12/29/23.
//

#pragma once

#include "is_buffer.hpp"

namespace fgl::engine::memory
{

	template < typename T >
	concept is_suballocation = requires( T t ) {
		{
			t.getBuffer()
		} -> is_buffer_ref;
		{
			t.getVkBuffer()
		} -> std::same_as< vk::Buffer >;
		{
			t.getOffset()
		} -> std::same_as< vk::DeviceSize >;
		/*
		{
			t.size()
		} -> std::same_as< vk::DeviceSize >;
		*/
		{
			t.descriptorInfo()
		} -> std::same_as< vk::DescriptorBufferInfo >;
	};
} // namespace fgl::engine::memory