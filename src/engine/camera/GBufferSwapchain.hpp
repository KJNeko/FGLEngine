//
// Created by kj16609 on 7/11/24.
//

#pragma once

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/PresentSwapChain.hpp"
#include "engine/rendering/RenderingFormats.hpp"
#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{

	constexpr std::size_t COLOR_INDEX { 0 };
	constexpr std::size_t POSITION_INDEX { 1 };
	constexpr std::size_t NORMAL_INDEX { 2 };
	constexpr std::size_t METALLIC_INDEX { 3 };
	constexpr std::size_t EMISSIVE_INDEX { 4 };

	constexpr std::size_t COMPOSITE_INDEX { 5 };
	constexpr std::size_t DEPTH_INDEX { 6 };

	class GBufferSwapchain
	{
		struct
		{
			ColorAttachment< COLOR_INDEX > m_color { pickColorFormat() };
			ColorAttachment< POSITION_INDEX > m_position { pickPositionFormat() };
			ColorAttachment< NORMAL_INDEX > m_normal { pickNormalFormat() };
			ColorAttachment< METALLIC_INDEX > m_metallic { pickMetallicFormat() };
			ColorAttachment< EMISSIVE_INDEX > m_emissive { pickEmissiveFormat() };
			//TODO: Move depth into m_position A channel
			DepthAttachment< DEPTH_INDEX > m_depth { pickDepthFormat() };

			// ColorAttachment< COMPOSITE_INDEX > m_composite { pickCompositeFormat() };
		} m_gbuffer {};

	  private:

		vk::Extent2D m_extent;

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > m_gbuffer_descriptor_set {};

		std::vector< std::unique_ptr< descriptors::DescriptorSet > > createGBufferDescriptors();

	  public:

		enum StageID : std::uint16_t
		{
			INITAL,
			COMPOSITE,
			FINAL
		};

		void transitionImages( vk::raii::CommandBuffer& command_buffer, std::uint16_t stage_id, FrameIndex index );
		vk::RenderingInfo getRenderingInfo( const FrameIndex frame_index );

		GBufferSwapchain( vk::Extent2D extent );
		~GBufferSwapchain();

		descriptors::DescriptorSet& getGBufferDescriptor( FrameIndex frame_index );

		vk::Extent2D getExtent() const;

		float getAspectRatio();
	};

} // namespace fgl::engine