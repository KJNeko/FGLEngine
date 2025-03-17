//
// Created by kj16609 on 3/17/25.
//
#pragma once
#include <glm/mat4x4.hpp>

#include <memory>

#include "memory/buffers/vector/DeviceVector.hpp"
#include "primitives/Transform.hpp"
#include "rendering/PresentSwapChain.hpp"

namespace fgl::engine
{
	class Model;

	struct ModelGPUInstance
	{
		//! Index of the model
		std::uint32_t m_model_index;
		glm::mat4x4 m_transform;
	};

	using InstanceIndex = std::uint32_t;

	using namespace fgl::literals::size_literals;

	class ModelInstance
	{
		std::shared_ptr< Model > m_model;
		InstanceIndex m_index;

		//! CPU side data to be modified
		ModelGPUInstance m_cpu_data;

		//! True if the last frame changed this instance in any way
		bool m_updated { false };

	  public:

		//! Returns GPU instance data
		ModelGPUInstance gpuInstanceData() const
		{
			ModelGPUInstance data {};

			data.m_model_index = m_model->getGPUID();

			return data;
		}

		//! Returns the current update state and sets it to false if it was true.
		bool acquireNeedsUpdate()
		{
			if ( m_updated ) [[unlikely]]
			{
				m_updated = false;
				return true;
			}

			return false;
		}
	};

} // namespace fgl::engine