//
// Created by kj16609 on 5/24/24.
//

#pragma once
#include "engine/FrameInfo.hpp"
#include "engine/camera/CameraDescriptor.hpp"
#include "engine/descriptors/DescriptorSetCollection.hpp"
#include "engine/rendering/pipelines/PipelineT.hpp"
#include "engine/rendering/pipelines/Shader.hpp"

namespace fgl::engine
{

	using StandardPipelineVertexShader = VertexShaderT< "shaders/textureless-gbuffer.vert" >;
	using StandardPipelineFragShader = FragmentShaderT< "shaders/textureless-gbuffer.frag" >;
	using StandardPipelineShaders = ShaderCollection< StandardPipelineVertexShader, StandardPipelineFragShader >;

	// using StandardPipelineDescriptorSets = descriptors::DescriptorSetCollection< GlobalDescriptorSet >;
	using StandardPipelineDescriptorSets =
		descriptors::DescriptorSetCollection< descriptors::EmptyDescriptorSet< 0 >, CameraDescriptorSet >;

	//! The standard pipeline is used for models without any form of texturing. They instead rely on Vertex coloring. A UV map is **NOT** expected
	using StandardPipeline = PipelineT< StandardPipelineShaders, StandardPipelineDescriptorSets >;

} // namespace fgl::engine
