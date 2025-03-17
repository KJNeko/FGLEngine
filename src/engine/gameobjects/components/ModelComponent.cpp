//
// Created by kj16609 on 3/1/25.
//

#include "ModelComponent.hpp"

#include "engine/assets/model/Model.hpp"

namespace fgl::engine::components
{

	ModelComponent::ModelComponent( std::shared_ptr< Model >&& model ) :
	  m_model( std::forward< decltype( m_model ) >( model ) ),
	  m_render_handle( m_model->getRenderHandle() )
	{}

} // namespace fgl::engine::components
