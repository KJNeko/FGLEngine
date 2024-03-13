//
// Created by kj16609 on 12/7/23.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace fgl::engine
{

	template < typename T, vk::ShaderStageFlags stages, std::uint16_t offset = 0 >
	struct PushConstant
	{
		using Type = T;

		constexpr static vk::PushConstantRange m_range { stages, offset, sizeof( T ) };

		PushConstant()
		{
			static_assert( sizeof( T ) <= 128, "Push constant range size must be less or equal to 128 bytes" );
		}

		static void push( vk::CommandBuffer command_buffer, vk::PipelineLayout m_pipeline_layout, T& data )
		{
			command_buffer.pushConstants( m_pipeline_layout, stages, offset, sizeof( T ), &data );
		}
	};



} // namespace fgl::engine