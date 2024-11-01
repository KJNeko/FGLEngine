//
// Created by kj16609 on 6/20/24.
//

#include "QueuePool.hpp"

#include "pipelines/Attachment.hpp"

namespace fgl::engine
{

	QueuePool::QueuePool( const vk::raii::PhysicalDevice& physical_device, Surface& surface )
	{
		const auto family_props { physical_device.getQueueFamilyProperties() };

		for ( std::uint32_t i = 0; i < family_props.size(); ++i )
		{
			auto& props { family_props[ i ] };
			if ( props.queueCount > 0 )
			{
				const vk::Bool32 can_present { physical_device.getSurfaceSupportKHR( i, surface ) };

				queue_info.emplace_back( props, can_present == VK_TRUE, 0 );
			}
		}
	}

	QueuePool::QueueIndex QueuePool::getIndex( const vk::QueueFlags flags, const vk::QueueFlags anti_flags )
	{
		for ( std::uint32_t i = 0; i < queue_info.size(); ++i )
		{
			const auto& [ props, can_present, num_allocated ] = queue_info[ i ];

			if ( ( props.queueFlags & flags ) && !( anti_flags & flags ) && props.queueCount > 0 )
			{
				return i;
			}
		}

		throw std::runtime_error( "Failed to get index of queue family with given flags" );
	}

	std::uint32_t QueuePool::getPresentIndex() const
	{
		for ( std::uint32_t i = 0; i < queue_info.size(); ++i )
		{
			if ( queue_info[ i ].can_present ) return i;
		}

		throw std::runtime_error( "Unable to find present queue" );
	}

} // namespace fgl::engine
