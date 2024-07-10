//
// Created by kj16609 on 1/2/24.
//

#pragma once

namespace fgl::engine
{
	class ImageView;
	class Image;
	struct AttachmentResources;

	template < typename T >
	concept is_attachment = requires( T a ) {
		{
			a.desc()
		} -> std::same_as< vk::AttachmentDescription& >;
		{
			a.getIndex()
		} -> std::same_as< std::uint32_t >;
		{
			a.setIndex( std::declval< std::uint32_t >() )
		};
		{
			a.linkImages( std::declval< std::vector< std::shared_ptr< Image > >& >() )
		};
		{
			a.view( std::declval< std::uint32_t >() )
		} -> std::same_as< ImageView& >;
		{
			a.m_clear_value
		} -> std::same_as< vk::ClearValue& >;
	};

	template < typename T >
	concept is_depth_attachment = is_attachment< T > && T::Layout == vk::ImageLayout::eDepthStencilAttachmentOptimal;

	template < typename T >
	concept is_color_attachment = is_attachment< T > && T::Layout == vk::ImageLayout::eColorAttachmentOptimal;
} // namespace fgl::engine
