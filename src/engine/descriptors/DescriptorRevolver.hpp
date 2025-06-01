//
// Created by kj16609 on 4/28/25.
//
#pragma once
#include <bitset>

#include "DescriptorSet.hpp"
#include "DescriptorSetLayout.hpp"
#include "assets/image/ImageView.hpp"
#include "assets/texture/Texture.hpp"
#include "constants.hpp"
#include "rendering/types.hpp"

namespace fgl::engine::descriptors
{

	struct BindingInfo;

	/**
	 * @brief Stores and automatically updates descriptors before use in a frame, Stores a unique descriptor per frame. Accumulates writes to the next descriptor use.
	 */
	class DescriptorRevolver : public std::enable_shared_from_this< DescriptorRevolver >
	{
		std::shared_ptr< DescriptorSetLayout > m_layout;

		mutable std::array< DescriptorSetPtr, constants::MAX_FRAMES_IN_FLIGHT > m_sets;
		mutable std::bitset< constants::MAX_FRAMES_IN_FLIGHT > m_updated;
		std::array< std::vector< BindingInfo >, constants::MAX_FRAMES_IN_FLIGHT > m_updates;
		mutable std::bitset< constants::MAX_FRAMES_IN_FLIGHT > m_set_updated;

		bool updateRequired( FrameIndex index ) const;

		void update( FrameIndex index );

		explicit DescriptorRevolver( DescriptorSetLayout& layout );

	  public:

		FGL_DEFAULT_COPY( DescriptorRevolver );
		FGL_DEFAULT_MOVE( DescriptorRevolver );
		// DescriptorRevolver();
		~DescriptorRevolver();

		static std::shared_ptr< DescriptorRevolver > create( DescriptorSetLayout& layout );

		DescriptorSetPtr get( FrameIndex index );

		DescriptorSetPtr operator[]( const FrameIndex index ) { return get( index ); }

		void bindImage(
			DescriptorIDX descriptor_idx, BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout );
		void bindBuffer(
			DescriptorIDX descriptor_idx, BindingIDX binding_idx, const memory::BufferSuballocation& buffer );
		void bindArray(
			DescriptorIDX descriptor_idx,
			BindingIDX binding_idx,
			const memory::BufferSuballocation& buffer,
			std::size_t array_idx,
			std::size_t item_size );
		void bindAttachment(
			DescriptorIDX descriptor_idx, BindingIDX binding_idx, const ImageViewPtr& view, vk::ImageLayout layout );
		void bindTexture( DescriptorIDX descriptor_idx, BindingIDX binding_idx, const TexturePtr& tex_ptr );
	};
} // namespace fgl::engine::descriptors