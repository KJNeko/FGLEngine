//
// Created by kj16609 on 2/28/25.
//
#pragma once
#include <memory>

#include "memory/buffers/vector/HostVector.hpp"
#include "rendering/pipelines/v2/Pipeline.hpp"

namespace fgl::engine
{
	struct FrameInfo;
	struct ModelMatrixInfo;

	class ShadowRenderer
	{
		std::unique_ptr< Pipeline > m_pipeline {};

		using DrawParameterBuffer = HostVector< vk::DrawIndexedIndirectCommand >;
		using DrawIndexedIndirectCommand = HostVector< ModelMatrixInfo >;

	  public:

		CommandBuffer& setupSystem( const FrameInfo& info );
		void pass( FrameInfo& info );

		ShadowRenderer();
	};

} // namespace fgl::engine