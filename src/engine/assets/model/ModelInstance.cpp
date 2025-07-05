//
// Created by kj16609 on 3/17/25.
//
#include "ModelInstance.hpp"

namespace fgl::engine
{
	ModelInstance::ModelInstance(
		std::vector< PrimitiveInstanceInfoIndex >&& primative_instances,
		ModelInstanceInfoIndex&& model_instance,
		const std::shared_ptr< Model >& model ) noexcept :
	  m_model( model ),
	  m_model_instance( std::move( model_instance ) ),
	  m_primitive_instances( std::move( primative_instances ) )
	{}

	bool ModelInstance::acquireNeedsUpdate()
	{
		if ( m_updated ) [[unlikely]]
		{
			m_updated = false;
			return true;
		}

		return false;
	}

	void ModelInstance::setTransform( const WorldTransform& transform )
	{
		ModelInstanceInfo info {};
		info.m_model_matrix = transform.mat4();
		info.m_normal_matrix = glm::transpose( glm::inverse( info.m_model_matrix ) );

		m_model_instance.update( info );

		m_updated = true;
	}
} // namespace fgl::engine