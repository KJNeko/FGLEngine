//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include "concepts.hpp"
#include "engine/FrameInfo.hpp"
#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/rendering/pipelines/PipelineT.hpp"
#include "engine/rendering/pipelines/Shader.hpp"

namespace fgl::engine
{

	class CompositionSystem
	{
		using DescriptorSets = descriptors::DescriptorSetCollection< GBufferDescriptorSet >;

		using VertexShader = VertexShaderT< "shaders/fullscreen.vert" >;
		using FragmentShader = FragmentShaderT< "shaders/composition.frag" >;

		using Shaders = ShaderCollection< VertexShader, FragmentShader >;

		using CompositionPipeline = PipelineT< Shaders, DescriptorSets >;

		std::unique_ptr< CompositionPipeline > m_composite_pipeline { nullptr };

		vk::raii::CommandBuffer& setupSystem( FrameInfo& info );

	  public:

		CompositionSystem( Device& device, vk::raii::RenderPass& render_pass );
		~CompositionSystem() = default;

		void pass( FrameInfo& info );
	};

	static_assert( is_system< CompositionSystem > );

} // namespace fgl::engine
