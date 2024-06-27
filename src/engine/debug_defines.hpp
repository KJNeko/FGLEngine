//
// Created by kj16609 on 6/25/24.
//

#pragma once

#define FULL_DEBUG_BARRIER( buffer )                                                                                   \
	{                                                                                                                  \
		vk::MemoryBarrier memory_barrier {};                                                                           \
		memory_barrier.srcAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead;             \
		memory_barrier.dstAccessMask = vk::AccessFlagBits::eMemoryWrite | vk::AccessFlagBits::eMemoryRead;             \
		std::vector< vk::MemoryBarrier > barriers { memory_barrier };                                                  \
                                                                                                                       \
		buffer.pipelineBarrier(                                                                                        \
			vk::PipelineStageFlagBits::eAllCommands,                                                                   \
			vk::PipelineStageFlagBits::eAllCommands,                                                                   \
			vk::DependencyFlagBits::eByRegion,                                                                         \
			barriers,                                                                                                  \
			{},                                                                                                        \
			{} );                                                                                                      \
	}
