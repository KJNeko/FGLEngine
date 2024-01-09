//
// Created by kj16609 on 1/1/24.
//

#pragma once

#include <concepts>
#include <cstdint>

namespace fgl::engine
{

	/**
	 * Descriptor must have a `static constexpr bool is_empty` member with the value `true`
	 * Descriptor must also be valid in `is_descriptor`
	 * @tparam T
	 */
	template < typename T >
	concept is_empty_descriptor = requires( T t ) {
		{
			t.m_binding_idx
		} -> std::same_as< const std::uint16_t& >;
		{
			t.is_empty
		} -> std::same_as< const bool& >;
	} && T::is_empty;

	template < typename T >
	concept is_layout_descriptor = requires( T t ) {
		{
			t.m_binding_idx
		} -> std::same_as< const std::uint16_t& >;
		{
			t.m_layout_binding
		} -> std::same_as< const vk::DescriptorSetLayoutBinding& >;
		{
			t.m_descriptor_type
		} -> std::same_as< const vk::DescriptorType& >;
	};

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