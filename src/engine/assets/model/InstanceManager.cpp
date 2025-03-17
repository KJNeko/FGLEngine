//
// Created by kj16609 on 3/17/25.
//

#include "InstanceManager.hpp"

namespace fgl::engine
{
	using namespace fgl::literals::size_literals;

	inline InstanceArray createInstances( memory::Buffer& buffer )
	{
		InstanceArray instances {};

		std::ranges::generate( instances, [ & ]() -> InstanceVector { return InstanceVector( buffer, 0 ); } );

		return instances;
	}

	inline InstanceManager::InstanceManager() :
	  m_buffer( 128_MiB, vk::BufferUsageFlagBits::eStorageBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal ),
	  m_instances( createInstances( m_buffer ) )
	{}

} // namespace fgl::engine