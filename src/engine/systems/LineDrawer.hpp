//
// Created by kj16609 on 8/10/24.
//

#pragma once
#include "engine/camera/CameraDescriptor.hpp"
#include "engine/descriptors/Descriptor.hpp"
#include "engine/memory/buffers/vector/HostVector.hpp"
#include "engine/rendering/SwapChain.hpp"
#include "engine/rendering/pipelines/v2/Pipeline.hpp"

namespace fgl::engine
{
	struct VertexLine;
	struct FrameInfo;
	struct ModelVertex;

	class LineDrawer
	{
		/*

		using VertexShader = VertexShaderT< "shaders/line.vert" >;
		using FragmentShader = FragmentShaderT< "shaders/line.frag" >;

		using LinePipeline = PipelineT<
			ShaderCollection< VertexShader, FragmentShader >,
			descriptors::DescriptorSetCollection< descriptors::EmptyDescriptorSet< 0 >, CameraDescriptorSet > >;

		*/


		std::unique_ptr< Pipeline > m_pipeline {};

		PerFrameArray< std::unique_ptr< HostVector< VertexLine > > > m_line_vertex_buffer {};

	  public:

		FGL_DELETE_ALL_Ro5( LineDrawer );

		vk::raii::CommandBuffer& setupSystem( FrameInfo& info );
		void pass( FrameInfo& info );

		LineDrawer( Device& device, vk::raii::RenderPass& render_pass );

		~LineDrawer();
	};

} // namespace fgl::engine