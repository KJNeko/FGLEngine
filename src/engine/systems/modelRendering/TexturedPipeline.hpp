//
// Created by kj16609 on 5/24/24.
//

#pragma once
#include "engine/FrameInfo.hpp"
#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/pipeline/Shader.hpp"

namespace fgl::engine
{

	using TexturedPipelineVertexShader = VertexShaderT< "shaders/textured-gbuffer.vert.spv" >;
	using TexturedPipelineFragShader = FragmentShaderT< "shaders/textured-gbuffer.frag.spv" >;
	using TexturedPipelineShaders = ShaderCollection< TexturedPipelineVertexShader, TexturedPipelineFragShader >;

	using TexturedPipelineDescriptorSets =
		descriptors::DescriptorSetCollection< GlobalDescriptorSet, TextureDescriptorSet >;

	//! The standard pipeline is used for models without any form of texturing. They instead rely on Vertex coloring. A UV map is **NOT** expected
	using TexturedPipeline = PipelineT< TexturedPipelineShaders, TexturedPipelineDescriptorSets >;

} // namespace fgl::engine
