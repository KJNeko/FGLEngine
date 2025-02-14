//
// Created by kj16609 on 6/23/24.
//

#pragma once
#include "engine/FrameInfo.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"
#include "engine/rendering/pipelines/Shader.hpp"

namespace fgl::engine
{
	class Pipeline;
	struct FrameInfo;

	/**
	 * @brief Handles the ImGui instances and rendering process. Takes the composite output from the CompositionSystem and renders it to the final output buffer to be presented
	 */
	class GuiSystem
	{
		std::unique_ptr< Pipeline > m_pipeline {};

		//Setup isn't needed for this. So we can just never define this safely.
		[[maybe_unused]] CommandBuffer& setupSystem( FrameInfo& info );

	  public:

		GuiSystem();

		FGL_DELETE_MOVE( GuiSystem );
		FGL_DELETE_COPY( GuiSystem );

		void pass( FrameInfo& info );
	};
} // namespace fgl::engine