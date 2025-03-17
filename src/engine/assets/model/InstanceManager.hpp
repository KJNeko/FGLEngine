//
// Created by kj16609 on 3/17/25.
//
#pragma once
#include "memory/buffers/Buffer.hpp"
#include "memory/buffers/vector/DeviceVector.hpp"
#include "rendering/PresentSwapChain.hpp"

namespace fgl::engine
{
	struct ModelGPUInstance;

	using InstanceVector = DeviceVector< ModelGPUInstance >;
	using InstanceArray = PerFrameArray< InstanceVector >;

	class InstanceManager
	{
		//! Buffer for each instance to use
		memory::Buffer m_buffer;

		InstanceArray m_instances;

	  public:

		//! Constructor to initialize m_buffer and m_instances
		[[nodiscard]] InstanceManager();

		~InstanceManager() = default;
	};

} // namespace fgl::engine