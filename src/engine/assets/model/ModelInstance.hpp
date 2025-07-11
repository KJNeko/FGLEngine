//
// Created by kj16609 on 3/17/25.
//
#pragma once

#include <memory>

#include "ModelInstanceInfo.hpp"
#include "Primitive.hpp"

namespace fgl::engine
{
	namespace components
	{
		class ModelComponent;
	}

	class Model;

	using InstanceIndex = std::uint32_t;

	class ModelInstance
	{
		std::shared_ptr< Model > m_model;

		ModelInstanceInfoIndex m_model_instance;
		std::vector< PrimitiveInstanceInfoIndex > m_primitive_instances;

		//! True if the last frame changed this instance in any way
		bool m_updated { false };

		friend class components::ModelComponent;

	  public:

		ModelInstance(
			std::vector< PrimitiveInstanceInfoIndex >&& primative_instances,
			ModelInstanceInfoIndex&& model_instance,
			const std::shared_ptr< Model >& model ) noexcept;

		void flagUpdate() { m_updated = true; }

		//! Returns the current update state and sets it to false if it was true.
		bool acquireNeedsUpdate();

		void setTransform( const WorldTransform& transform );
	};

} // namespace fgl::engine