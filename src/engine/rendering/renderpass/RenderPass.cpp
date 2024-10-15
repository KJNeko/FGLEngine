//
// Created by kj16609 on 7/21/24.
//

#include "RenderPass.hpp"

#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine::rendering
{

	SubpassBuilder::SubpassBuilder( RenderPassBuilder& builder, const std::uint32_t index ) :
	  m_builder( builder ),
	  m_subpass_idx( index )
	{}

	//! Sets the layout the attachment is expected to and should be in during this subpass
	void SubpassBuilder::addRenderLayout( const std::uint32_t index, const vk::ImageLayout layout )
	{
		// Check that the layout hasn't already been set
		assert(
			std::find_if(
				m_attachment_references.begin(),
				m_attachment_references.end(),
				[ index ]( const vk::AttachmentReference& attachment_ref )
				{ return attachment_ref.attachment == index; } )
			== m_attachment_references.end() );

		assert( layout != vk::ImageLayout::eShaderReadOnlyOptimal );
		assert( layout != vk::ImageLayout::ePreinitialized );
		assert( layout != vk::ImageLayout::eDepthStencilAttachmentOptimal );

		m_attachment_references.emplace_back( vk::AttachmentReference( index, layout ) );
	}

	void SubpassBuilder::addInputLayout( const std::uint32_t index, const vk::ImageLayout layout )
	{
		// Check that the layout hasn't already been set
		assert(
			std::find_if(
				m_input_attachment_references.begin(),
				m_input_attachment_references.end(),
				[ index ]( const vk::AttachmentReference& attachment_ref )
				{ return attachment_ref.attachment == index; } )
			== m_input_attachment_references.end() );

		m_input_attachment_references.emplace_back( vk::AttachmentReference( index, layout ) );
	}

	void SubpassBuilder::setDepthLayout( const std::uint32_t index, const vk::ImageLayout layout )
	{
		m_depth_reference = vk::AttachmentReference( index, layout );
	}

	void SubpassBuilder::addDependency(
		const std::uint32_t src_idx,
		const std::uint32_t dst_idx,
		const vk::AccessFlags src_access,
		const vk::PipelineStageFlags src_stage,
		const vk::AccessFlags dst_access,
		const vk::PipelineStageFlags dst_stage,
		const vk::DependencyFlags extra_flags )
	{
		vk::SubpassDependency info {};
		info.srcSubpass = src_idx;
		info.dstSubpass = dst_idx;

		info.srcAccessMask = src_access;
		info.srcStageMask = src_stage;

		assert( dst_stage != vk::PipelineStageFlags( 0 ) );

		info.dstAccessMask = dst_access;
		info.dstStageMask = dst_stage;

		info.dependencyFlags = extra_flags;

		m_dependencies.emplace_back( info );
	}

	void SubpassBuilder::addDependency(
		const SubpassBuilder& source,
		const vk::AccessFlags src_access,
		const vk::PipelineStageFlags src_stage,
		const vk::AccessFlags dst_access,
		const vk::PipelineStageFlags dst_stage,
		const vk::DependencyFlags extra_flags )
	{
		return addDependency(
			source.m_subpass_idx, m_subpass_idx, src_access, src_stage, dst_access, dst_stage, extra_flags );
	}

	void SubpassBuilder::addDependencyToExternal(
		const vk::AccessFlags src_access,
		const vk::PipelineStageFlags src_stage,
		const vk::AccessFlags dst_access,
		const vk::PipelineStageFlags dst_stage,
		const vk::DependencyFlags extra_flags )
	{
		return addDependency(
			m_subpass_idx,
			VK_SUBPASS_EXTERNAL,
			src_access,
			src_stage,
			( dst_access == vk::AccessFlagBits::eNone ) ? src_access : dst_access,
			( dst_stage == vk::PipelineStageFlagBits::eNone ) ? src_stage : dst_stage,
			extra_flags );
	}

	void SubpassBuilder::addDependencyFromExternal(
		const vk::AccessFlags src_access,
		const vk::PipelineStageFlags src_stage,
		const vk::AccessFlags dst_access,
		const vk::PipelineStageFlags dst_stage,
		const vk::DependencyFlags extra_flags )
	{
		return addDependency(
			VK_SUBPASS_EXTERNAL,
			m_subpass_idx,
			src_access,
			src_stage,
			( dst_access == vk::AccessFlagBits::eNone ) ? src_access : dst_access,
			( dst_stage == vk::PipelineStageFlagBits::eNone ) ? src_stage : dst_stage,
			extra_flags );
	}

	vk::SubpassDescription SubpassBuilder::description() const
	{
		vk::SubpassDescription description {};

		description.setInputAttachments( m_input_attachment_references );
		description.setColorAttachments( m_attachment_references );
		description.setPDepthStencilAttachment( &m_depth_reference );

		// We'll need to allow this to be changed if we are gonna use compute alongside graphics during a render pass
		description.setPipelineBindPoint( vk::PipelineBindPoint::eGraphics );

		// Apparently this can be used to preserve an attachment from a previous subpass.
		// This might be helpful to ensure we have set if we aren't using an attachment in a certian subpass
		// But use it later in the subpass.
		description.setPreserveAttachments( m_preserved_attachment_references );

		//todo: Figure out what resolve attachments are for.

		// Flags aren't needed unless we are doing something with a specific device.
		description.flags = vk::SubpassDescriptionFlags( 0 );

		return description;
	}

	AttachmentInfo::AttachmentInfo( RenderPassBuilder& builder, std::size_t index ) :
	  m_index( index ),
	  m_builder( builder )
	{}

	void AttachmentInfo::setFormat( vk::Format format )
	{
		m_builder.setFormat( m_index, format );
#ifndef NDEBUG
		assert( !set_format );
		set_format = true;
#endif
	}

	void AttachmentInfo::setLayouts( vk::ImageLayout image_layout, vk::ImageLayout final_layout )
	{
		m_builder.setLayouts( m_index, image_layout, final_layout );
#ifndef NDEBUG
		assert( !set_layout );
		set_layout = true;
#endif
	}

	void AttachmentInfo::setOps( vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op )
	{
		m_builder.setOps( m_index, load_op, store_op );
#ifndef NDEBUG
		assert( !set_ops );
		set_ops = true;
#endif
	}

	SubpassBuilder& RenderPassBuilder::createSubpass( const std::uint32_t index )
	{
		m_subpasses.emplace_back( new SubpassBuilder( *this, index ) );
		return *m_subpasses.back();
	}

	SubpassBuilder& RenderPassBuilder::subpass( const std::size_t index ) const
	{
		return *m_subpasses[ index ];
	}

	void RenderPassBuilder::setAttachmentCount( const std::size_t count )
	{
		m_attachment_descriptions.resize( count );
	}

	void RenderPassBuilder::setFormat( const std::size_t index, const vk::Format format )
	{
		assert( index < m_attachment_descriptions.size() && "Maybe forgot call setAttachmentCount()?" );
		m_attachment_descriptions[ index ].format = format;
	}

	void RenderPassBuilder::setInitalLayout( const std::size_t index, const vk::ImageLayout layout )
	{
		assert( index < m_attachment_descriptions.size() && "Maybe forgot call setAttachmentCount()?" );
		m_attachment_descriptions[ index ].initialLayout = layout;
	}

	void RenderPassBuilder::setFinalLayout( const std::size_t index, const vk::ImageLayout layout )
	{
		assert( index < m_attachment_descriptions.size() && "Maybe forgot call setAttachmentCount()?" );
		m_attachment_descriptions[ index ].finalLayout = layout;
	}

	void RenderPassBuilder::
		setOps( const std::size_t index, const vk::AttachmentLoadOp load_op, const vk::AttachmentStoreOp store_op )
	{
		m_attachment_descriptions[ index ].loadOp = load_op;
		m_attachment_descriptions[ index ].storeOp = store_op;
	}

	void RenderPassBuilder::
		setLayouts( const std::size_t index, const vk::ImageLayout inital_layout, const vk::ImageLayout final_layout )
	{
		setInitalLayout( index, inital_layout );
		setFinalLayout( index, final_layout );
	}

	vk::raii::RenderPass RenderPassBuilder::create()
	{
		vk::RenderPassCreateInfo info {};

		std::vector< vk::SubpassDescription > subpass_descriptions {};
		std::vector< vk::SubpassDependency > subpass_dependencies {};

		for ( const auto& subpass : m_subpasses )
		{
			subpass_descriptions.emplace_back( subpass->description() );

			for ( const auto& dependency : subpass->dependencies() )
			{
				subpass_dependencies.emplace_back( dependency );
			}
		}

		info.setSubpasses( subpass_descriptions );
		// info.setSubpassCount( subpass_descriptions.size() );
		info.setDependencies( subpass_dependencies );
		info.setAttachments( m_attachment_descriptions );

		return Device::getInstance()->createRenderPass( info );
	}

	AttachmentInfo RenderPassBuilder::attachment( const std::size_t index )
	{
		return AttachmentInfo( *this, index );
	}
} // namespace fgl::engine::rendering
