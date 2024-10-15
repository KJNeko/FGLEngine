//
// Created by kj16609 on 10/9/24.
//

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/rendering/pipelines/Pipeline.hpp"

namespace fgl::engine

{
	namespace descriptors
	{
		class DescriptorSet;
	}

	class Pipeline
	{
		vk::raii::Pipeline m_pipeline;
		vk::raii::PipelineLayout m_layout;

	  public:

		Pipeline() = delete;
		Pipeline( vk::raii::Pipeline&& pipeline, vk::raii::PipelineLayout&& layout );

		void bind( vk::raii::CommandBuffer& );

		void bindDescriptor( vk::raii::CommandBuffer&, descriptors::DescriptorIDX descriptor_idx, descriptors::DescriptorSet& set );
		void bindDescriptor( vk::raii::CommandBuffer& comd_buffer, descriptors::DescriptorSet& set );

		void setDebugName( const char* str );
	};

} // namespace fgl::engine
