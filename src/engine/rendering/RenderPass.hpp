//
// Created by kj16609 on 7/21/24.
//

#pragma once
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include <vector>

namespace fgl::engine::rendering
{
	class RenderPassBuilder;

	class SubpassBuilder
	{
		RenderPassBuilder& m_builder;

		std::uint32_t m_subpass_idx;
		std::vector< vk::AttachmentReference > m_attachment_references {};
		std::vector< vk::AttachmentReference > m_input_attachment_references {};
		std::vector< std::uint32_t > m_preserved_attachment_references {};
		vk::AttachmentReference m_depth_reference { VK_ATTACHMENT_UNUSED, vk::ImageLayout::eUndefined };
		std::vector< vk::SubpassDependency > m_dependencies {};

		SubpassBuilder( RenderPassBuilder& builder, std::uint32_t index );

	  public:

		// BEGIN Attachments

		//! Sets the expected attachment layout for this subpass at the index given.
		void addRenderLayout( std::uint32_t index, vk::ImageLayout layout );

		void addInputLayout( std::uint32_t index, vk::ImageLayout layout );
		void setDepthLayout( std::uint32_t index, vk::ImageLayout layout );

		//TODO: This
		//void setPreserved( const std::uint32_t index,  SubpassBuilder& subpass ) {}

		// END Attachments

		void addDependency(
			std::uint32_t src_idx,
			std::uint32_t dst_idx,
			vk::AccessFlags src_access,
			vk::PipelineStageFlags src_stage,
			vk::AccessFlags dst_access,
			vk::PipelineStageFlags dst_stage,
			vk::DependencyFlags extra_flags );

		void addDependency(
			const SubpassBuilder& source,
			vk::AccessFlags src_access,
			vk::PipelineStageFlags src_stage,
			vk::AccessFlags dst_access,
			vk::PipelineStageFlags dst_stage,
			vk::DependencyFlags extra_flags = vk::DependencyFlags( 0 ) );

		void addDependencyToExternal(
			vk::AccessFlags src_access,
			vk::PipelineStageFlags src_stage,
			vk::AccessFlags dst_access,
			vk::PipelineStageFlags dst_stage,
			vk::DependencyFlags extra_flags = vk::DependencyFlags( 0 ) );

		void addDependencyFromExternal(
			vk::AccessFlags src_access,
			vk::PipelineStageFlags src_stage,
			vk::AccessFlags dst_access = vk::AccessFlagBits::eNone,
			vk::PipelineStageFlags dst_stage = vk::PipelineStageFlagBits::eNone,
			vk::DependencyFlags extra_flags = vk::DependencyFlags( 0 ) );

		friend class RenderPassBuilder;

	  private:

		vk::SubpassDescription description() const;

		std::vector< vk::SubpassDependency > dependencies() { return m_dependencies; }
	};

	struct AttachmentInfo
	{
		std::size_t m_index;
		RenderPassBuilder& m_builder;

#ifndef NDEBUG
		bool set_ops { false };
		bool set_layout { false };
		bool set_format { false };
#endif

		AttachmentInfo( RenderPassBuilder& builder, std::size_t index );

		void setFormat( vk::Format format );

		void setLayouts( vk::ImageLayout image_layout, vk::ImageLayout final_layout );

		void setOps( vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op );
#ifndef NDEBUG
		~AttachmentInfo()
		{
			assert( set_ops );
			assert( set_layout );
			assert( set_format );
		}
#endif
	};

	class RenderPassBuilder
	{
		std::vector< std::unique_ptr< SubpassBuilder > > m_subpasses {};
		std::vector< vk::AttachmentDescription > m_attachment_descriptions {};

	  public:

		SubpassBuilder& createSubpass( std::uint32_t index );

		SubpassBuilder& subpass( std::size_t index ) const;

		void setAttachmentCount( std::size_t count );

		void setFormat( std::size_t index, vk::Format format );

		void setInitalLayout( std::size_t index, vk::ImageLayout layout );

		void setFinalLayout( std::size_t index, vk::ImageLayout layout );
		void setOps( std::size_t index, vk::AttachmentLoadOp load_op, vk::AttachmentStoreOp store_op );

		void setLayouts( std::size_t index, vk::ImageLayout inital_layout, vk::ImageLayout final_layout );

		vk::raii::RenderPass create();

		AttachmentInfo attachment( std::size_t index );
	};

	class RenderPass
	{
		vk::raii::RenderPass m_renderpass;

		std::vector< vk::AttachmentDescription >& getAttachmentDescriptions();
		std::vector< vk::SubpassDescription >& getSubpassDescriptions();
		std::vector< vk::SubpassDependency >& getSubpassDependencies();

		vk::raii::RenderPass createRenderpass();

	  public:

		RenderPass();
	};

} // namespace fgl::engine::rendering
