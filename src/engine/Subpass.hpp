//
// Created by kj16609 on 12/30/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

#include <array>
#include <cstdint>

#include "Attachment.hpp"
#include "engine/concepts/is_attachment.hpp"

namespace fgl::engine
{

	template < typename T >
	concept is_subpass = requires( T a ) {
		{
			a.description()
		} -> std::same_as< vk::SubpassDescription >;
	};

	template < typename T >
	using UnwrapInputAttachment = std::conditional_t< is_input_attachment< T >, typename T::Attachment, T >;

	template <
		vk::PipelineBindPoint bind_point,
		is_wrapped_attachment Attachment,
		is_wrapped_attachment... Attachments >
	class Subpass
	{
		//! Set to true if the first attachment is a valid depth stencil attachment
		constexpr static bool has_depth_stencil_attachment { Attachment::m_layout
			                                                 == vk::ImageLayout::eDepthStencilAttachmentOptimal };

		constexpr static std::uint32_t attachment_count { ( ( !is_input_attachment< Attachments > ? 1 : 0 ) + ... )
			                                              + ( has_depth_stencil_attachment ? 0 : 1 ) };

		constexpr static std::uint32_t input_attachments { ( ( is_input_attachment< Attachments > ? 1 : 0 ) + ... ) };

		std::uint32_t index;
		std::array< vk::AttachmentReference, attachment_count > attachment_references {};
		vk::AttachmentReference depth_stencil_reference {};

		std::array< vk::AttachmentReference, input_attachments > input_attachment_references {};

		vk::SubpassDescription subpass_description {};

		std::vector< vk::SubpassDependency > dependencies {};

		std::vector< vk::WriteDescriptorSet > m_descriptor_writes {};

		std::uint32_t current_input_idx { 0 };
		std::uint32_t current_attachment_idx { 0 };

		template < is_wrapped_attachment T >
		void registerAttachment( UnwrappedAttachment< T >& attachment )
		{
			if constexpr ( is_input_attachment< T > )
			{
				input_attachment_references[ current_input_idx++ ] = { attachment.getIndex(), T::m_layout };
			}
			else if constexpr ( is_used_attachment< T > )
			{
				attachment_references[ current_attachment_idx++ ] = { attachment.getIndex(), T::m_layout };
			}
		}

	  public:

		std::uint32_t getIndex() const { return index; }

		Subpass(
			const std::uint32_t idx,
			UnwrappedAttachment< Attachment >& first_attachment,
			UnwrappedAttachment< Attachments >&... attachments ) :
		  index( idx )
		{
			//TODO: Redo this check. As this will prevent any input attachments from being used as a depth input (Which may be done?)
			static_assert(
				( ( Attachments::m_layout != vk::ImageLayout::eDepthStencilAttachmentOptimal ) && ... ),
				"Depth stencil must be at attachment index 0 in template parameters" );

			if constexpr ( has_depth_stencil_attachment )
			{
				depth_stencil_reference.layout = Attachment::m_layout;
				depth_stencil_reference.attachment = first_attachment.getIndex();
			}
			else
			{
				registerAttachment< Attachment >( first_attachment );
			}

			( ( registerAttachment< Attachments >( attachments ) ), ... );

			subpass_description.pipelineBindPoint = bind_point;
			subpass_description.pColorAttachments = attachment_references.data();
			subpass_description.colorAttachmentCount = static_cast< std::uint32_t >( attachment_references.size() );

			subpass_description.pDepthStencilAttachment =
				has_depth_stencil_attachment ? &depth_stencil_reference : nullptr;

			subpass_description.pInputAttachments = input_attachment_references.data();
			subpass_description.inputAttachmentCount =
				static_cast< std::uint32_t >( input_attachment_references.size() );
		}

		vk::SubpassDescription description() { return subpass_description; }

		friend class RenderPass;

		void registerDependency(
			std::uint32_t src_subpass,
			std::uint32_t dst_subpass,
			vk::AccessFlags src_access_flags,
			vk::PipelineStageFlags src_stage_flags,
			vk::AccessFlags dst_access_flags,
			vk::PipelineStageFlags dst_stage_flags,
			const vk::DependencyFlags dependency_flags )
		{
			vk::SubpassDependency subpass_dependency {};
			subpass_dependency.srcSubpass = src_subpass;
			subpass_dependency.dstSubpass = dst_subpass;

			subpass_dependency.srcStageMask = src_stage_flags;
			subpass_dependency.srcAccessMask = src_access_flags;

			subpass_dependency.dstStageMask = dst_stage_flags;
			subpass_dependency.dstAccessMask = dst_access_flags;

			subpass_dependency.dependencyFlags = dependency_flags;

			dependencies.push_back( subpass_dependency );
		}

		template < is_subpass SrcT >
		void registerDependency(
			SrcT& parent,
			const vk::AccessFlags src_access_flags,
			const vk::PipelineStageFlags src_stage_flags,
			const vk::AccessFlags dst_access_flags,
			const vk::PipelineStageFlags dst_stage_flags,
			const vk::DependencyFlags dependency_flags )
		{
			registerDependency(
				parent.getIndex(),
				this->index,
				src_access_flags,
				src_stage_flags,
				dst_access_flags,
				dst_stage_flags,
				dependency_flags );
		}

		void registerExternalDependency(
			const vk::AccessFlags access_flags,
			const vk::PipelineStageFlags stage_flags,
			const vk::DependencyFlags dependency_flags = {} )
		{
			registerDependency(
				VK_SUBPASS_EXTERNAL,
				this->index,
				access_flags,
				stage_flags,
				vk::AccessFlags( 0 ),
				stage_flags,
				dependency_flags );
		}
	};

} // namespace fgl::engine
