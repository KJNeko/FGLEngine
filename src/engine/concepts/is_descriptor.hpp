//
// Created by kj16609 on 1/1/24.
//

#pragma once

#include "is_empty_descriptor.hpp"
#include "is_layout_descriptor.hpp"

namespace fgl::engine
{
	template < typename T >
	concept is_image_descriptor =
		is_layout_descriptor< T > && ( T::m_descriptor_type == vk::DescriptorType::eSampledImage );

	template < typename T >
	concept is_attachment_descriptor =
		is_layout_descriptor< T > && ( T::m_descriptor_type == vk::DescriptorType::eInputAttachment );

	template < typename T >
	concept is_uniform_buffer_desciptor =
		is_layout_descriptor< T > && ( T::m_descriptor_type == vk::DescriptorType::eUniformBuffer );

	template < typename T >
	concept is_storage_buffer_descriptor =
		is_layout_descriptor< T > && ( T::m_descriptor_type == vk::DescriptorType::eStorageBuffer );

	template < typename T >
	concept is_buffer_descriptor = is_uniform_buffer_desciptor< T > || is_storage_buffer_descriptor< T >;

	template < typename T > concept is_descriptor = is_empty_descriptor< T > || is_layout_descriptor< T >;
} // namespace fgl::engine