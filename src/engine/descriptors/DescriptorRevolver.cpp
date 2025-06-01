//
// Created by kj16609 on 4/28/25.
//
#include "DescriptorRevolver.hpp"

#include "assets/image/ImageView.hpp"
#include "memory/buffers/BufferSuballocationHandle.hpp"

namespace fgl::engine::descriptors
{
	DescriptorRevolver::~DescriptorRevolver()
	{}

	std::shared_ptr< DescriptorRevolver > DescriptorRevolver::create( DescriptorSetLayout& layout )
	{
		return std::shared_ptr< DescriptorRevolver >( new DescriptorRevolver( layout ) );
	}

	DescriptorRevolver::DescriptorRevolver( DescriptorSetLayout& layout ) :
	  m_layout(),
	  m_sets(),
	  m_updated( 0 ),
	  m_updates(),
	  m_set_updated( 0 )
	{
		for ( FrameIndex i = 0; i < constants::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			m_sets[ i ] = layout.create();
		}
	}

	DescriptorSetPtr DescriptorRevolver::get( const FrameIndex index )
	{
		if ( updateRequired( index ) ) update( index );

		return m_sets[ index ];
	}

	struct BufferBindingInfo
	{
		memory::FrozenBufferSuballocation m_handle;
	};

	struct ArrayBindingInfo
	{
		memory::FrozenBufferSuballocation m_handle;
		std::size_t m_array_idx;
		std::size_t m_item_size;
	};

	struct ImageBindingInfo
	{
		ImageViewPtr m_image_view;
		vk::ImageLayout m_layout;
	};

	struct AttachmentBindingInfo : public ImageBindingInfo
	{};

	struct TextureBindingInfo
	{
		TexturePtr m_texture;
	};

	using BindingVariant = std::
		variant< BufferBindingInfo, ArrayBindingInfo, ImageBindingInfo, AttachmentBindingInfo, TextureBindingInfo >;

	struct BindingInfo
	{
		BindingIDX m_binding_idx;
		BindingVariant m_data;
	};

	void DescriptorRevolver::
		bindBuffer( DescriptorIDX descriptor_idx, BindingIDX binding_idx, const memory::BufferSuballocation& buffer )
	{
		const BufferBindingInfo info { .m_handle = buffer.freeze() };

		m_updates[ descriptor_idx ].emplace_back( binding_idx, info );
	}

	void DescriptorRevolver::bindArray(
		const DescriptorIDX descriptor_idx,
		BindingIDX binding_idx,
		const memory::BufferSuballocation& buffer,
		const std::size_t array_idx,
		const std::size_t item_size )
	{
		const ArrayBindingInfo info { .m_handle = buffer.freeze(), .m_array_idx = array_idx, .m_item_size = item_size };

		m_updates[ descriptor_idx ].emplace_back( binding_idx, info );
	}

	void DescriptorRevolver::bindImage(
		const DescriptorIDX descriptor_idx,
		BindingIDX binding_idx,
		const ImageViewPtr& view,
		const vk::ImageLayout layout )
	{
		const ImageBindingInfo info { .m_image_view = view, .m_layout = layout };

		m_updates[ descriptor_idx ].emplace_back( binding_idx, info );
	}

	void DescriptorRevolver::bindAttachment(
		const DescriptorIDX descriptor_idx,
		BindingIDX binding_idx,
		const ImageViewPtr& view,
		const vk::ImageLayout layout )
	{
		ImageBindingInfo info { .m_image_view = view, .m_layout = layout };

		m_updates[ descriptor_idx ].emplace_back( binding_idx, info );
	}

	void DescriptorRevolver::bindTexture(
		const DescriptorIDX descriptor_idx, BindingIDX binding_idx, const std::shared_ptr< Texture >& tex_ptr )
	{
		TextureBindingInfo info { .m_texture = tex_ptr };

		m_updates[ descriptor_idx ].emplace_back( binding_idx, info );
	}

	void updateBufferInfo( const BindingInfo& bind_info, DescriptorSetPtr& set )
	{
		const auto& [ m_handle ] { std::get< BufferBindingInfo >( bind_info.m_data ) };

		set->bindBuffer( bind_info.m_binding_idx, std::move( m_handle ) );
	}

	void updateArrayInfo( const BindingInfo& bind_info, DescriptorSetPtr& set )
	{
		const auto& [ handle, array_idx, item_size ] { std::get< ArrayBindingInfo >( bind_info.m_data ) };

		set->bindArray( bind_info.m_binding_idx, std::move( handle ), array_idx, item_size );
	}

	void updateImageInfo( const BindingInfo& bind_info, DescriptorSetPtr& set )
	{
		const auto& [ image_view, layout ] { std::get< ImageBindingInfo >( bind_info.m_data ) };

		set->bindImage( bind_info.m_binding_idx, image_view, layout );
	}

	void updateAttachmentInfo( const BindingInfo& bind_info, DescriptorSetPtr& set )
	{
		const auto& [ image_view, layout ] { std::get< ImageBindingInfo >( bind_info.m_data ) };

		set->bindAttachment( bind_info.m_binding_idx, image_view, layout );
	}

	void updateTextureInfo( const BindingInfo& bind_info, const DescriptorSetPtr& set )
	{
		const auto& [ m_texture ] { std::get< TextureBindingInfo >( bind_info.m_data ) };

		set->bindTexture( bind_info.m_binding_idx, m_texture );
	}

	void processUpdates( const std::vector< BindingInfo >& binding_infos, DescriptorSetPtr& set )
	{
		for ( const BindingInfo& info : binding_infos )
		{
			std::visit(
				[ &set, &info ]< typename T0 >( [[maybe_unused]] T0&& arg ) -> void
				{
					using T = std::decay_t< T0 >;
					if constexpr ( std::same_as< T, BufferBindingInfo > )
						updateBufferInfo( info, set );
					else if constexpr ( std::same_as< T, ArrayBindingInfo > )
						updateArrayInfo( info, set );
					else if constexpr ( std::same_as< T, ImageBindingInfo > )
						updateImageInfo( info, set );
					else if constexpr ( std::same_as< T, AttachmentBindingInfo > )
						updateAttachmentInfo( info, set );
					else if constexpr ( std::same_as< T, TextureBindingInfo > )
						updateTextureInfo( info, set );
				},
				info.m_data );
		}

		set->update();
	}

	bool DescriptorRevolver::updateRequired( FrameIndex index ) const
	{
		return m_set_updated.test( index );
	}

	void DescriptorRevolver::update( const FrameIndex index )
	{
		m_set_updated.set( index, true );
		// If we have pending updates, then we need to update
		const bool has_updates { !m_updates[ index ].empty() };

		m_updated.set( index, has_updates );

		if ( m_updated.any() )
		{
			// One of the previous frames has updated since we last used it. We need to process the queue.

			// we start at the frame after this, Which will technically be the 'oldest' frame.
			FrameIndex cur_idx { static_cast< FrameIndex >( ( index + 1 ) % constants::MAX_FRAMES_IN_FLIGHT ) };

			do {
				// process the updates if that frame had an update to the descriptors
				if ( m_updated.test( cur_idx ) ) processUpdates( m_updates[ cur_idx ], m_sets[ index ] );

				cur_idx = static_cast< FrameIndex >( ( cur_idx + 1 ) % constants::MAX_FRAMES_IN_FLIGHT );
			}
			while ( cur_idx != index );
		}

		const auto next_index { ( index + 1 ) % constants::MAX_FRAMES_IN_FLIGHT };

		// Set the next descriptor to be not updated
		m_updated.set( next_index, false );
		m_set_updated.set( next_index, false );
		m_updates[ next_index ].clear();

		return;
	}

} // namespace fgl::engine::descriptors