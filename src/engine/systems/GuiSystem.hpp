//
// Created by kj16609 on 6/23/24.
//

#pragma once
#include "engine/FrameInfo.hpp"
#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/pipeline/Shader.hpp"

namespace fgl::engine
{
	struct FrameInfo;

	/**
	 * @brief Handles the ImGui instances and rendering process. Takes the composite output from the CompositionSystem and renders it to the final output buffer to be presented
	 */
	class GuiSystem
	{
		// Attachment 0 will be the composited image
		using CompositeDescriptorSets = descriptors::DescriptorSetCollection< GBufferCompositeDescriptorSet >;

		using VertexShader = VertexShaderT< "shaders/fullscreen.vert.spv" >;
		using FragmentShader = FragmentShaderT< "shaders/gui-compose.frag.spv" >;

		using Shaders = ShaderCollection< VertexShader, FragmentShader >;
		using Pipeline = PipelineT< Shaders, CompositeDescriptorSets >;

		std::unique_ptr< Pipeline > m_pipeline { nullptr };

		//Setup isn't needed for this. So we can just never define this safely.
		[[maybe_unused]] vk::raii::CommandBuffer& setupSystem( FrameInfo& info );

	  public:

		GuiSystem( Device& device, vk::raii::RenderPass& render_pass );

		FGL_DELETE_ALL_Ro5( GuiSystem );

		void pass( FrameInfo& info );
	};
} // namespace fgl::engine