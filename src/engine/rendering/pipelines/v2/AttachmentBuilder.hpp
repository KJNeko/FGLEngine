//
// Created by kj16609 on 10/10/24.
//

#include "engine/rendering/pipelines/Attachment.hpp"

namespace fgl::engine
{
	class PipelineBuilder;

	class AttachmentBuilder
	{
		PipelineBuilder& parent;
		bool m_finished { false };

		AttachmentBuilder( PipelineBuilder& source );
		AttachmentBuilder() = delete;

		friend class PipelineBuilder;

	  public:

		vk::PipelineColorBlendAttachmentState color_blend_config {};

		void finish();

		AttachmentBuilder& enableBlend();

		~AttachmentBuilder();
	};

} // namespace fgl::engine
