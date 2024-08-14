//
// Created by kj16609 on 1/2/24.
//

#pragma once

#include <memory>

#include "../rendering/pipelines/Attachment.hpp"

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
			a.m_index
		} -> std::same_as< const std::uint32_t& >;
		{
			a.attachImageView( std::declval< std::uint16_t >(), std::declval< std::shared_ptr< ImageView > >() )
		};
		{
			a.linkImage( std::declval< std::uint16_t >(), std::declval< Image& >() )
		};
		{
			a.getView( std::declval< std::uint8_t >() )
		} -> std::same_as< ImageView& >;
		{
			a.m_clear_value
		} -> std::same_as< vk::ClearValue& >;
	};

} // namespace fgl::engine
