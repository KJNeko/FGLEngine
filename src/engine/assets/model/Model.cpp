//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>

#include "EngineContext.hpp"
#include "ModelInstance.hpp"
#include "builders/ModelBuilder.hpp"
#include "builders/SceneBuilder.hpp"

namespace fgl::engine
{
	using namespace fgl::literals::size_literals;

	ModelGPUBuffers& getModelBuffers()
	{
		return EngineContext::getInstance().m_model_buffers;
	}

	ModelGPUBuffers::ModelGPUBuffers() :
	  m_long_buffer(
		  32_MiB,
		  vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal ),
	  m_short_buffer(
		  16_MiB,
		  vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_vertex_buffer(
		  2_MiB,
		  vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer
			  | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal ),
	  m_index_buffer(
		  1_GiB,
		  vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal ),
	  m_primitive_info( m_long_buffer ),
	  m_primitive_instances( m_short_buffer ),
	  m_model_instances( m_short_buffer ),
	  m_generated_instance_info( constructPerFrame< DeviceVector< PerVertexInstanceInfo > >( m_vertex_buffer ) )
	{
		m_primitives_desc = PRIMITIVE_SET.create();
		m_primitives_desc->bindStorageBuffer( 0, m_primitive_info );
		m_primitives_desc->update();
		m_primitives_desc->setName( "Primitives" );

		m_instances_desc = INSTANCES_SET.create();
		m_instances_desc->bindStorageBuffer( 0, m_primitive_instances );
		m_instances_desc->bindStorageBuffer( 1, m_model_instances );
		m_instances_desc->update();
		m_instances_desc->setName( "Instances, Primitive + Models" );
	}

	OrientedBoundingBox< CoordinateSpace::Model > Model::buildBoundingBox( const std::vector< Primitive >& primitives )
	{
		ZoneScoped;
		assert( !primitives.empty() );
		if ( primitives.empty() ) return {};

		OrientedBoundingBox< CoordinateSpace::Model > box { primitives.at( 0 ).m_bounding_box };

		for ( std::uint64_t i = 1; i < primitives.size(); i++ ) box = box.combine( primitives[ i ].m_bounding_box );

		return box;
	}

	bool Model::ready() const
	{
		//Return true if even a single primitive is ready
		for ( auto& primitive : this->m_primitives )
		{
			if ( primitive.ready() ) return true;
		}
		return false;
	}

	Model::Model( std::vector< Primitive >&& primitives, const std::string& name ) :
	  m_name( name ),
	  m_primitives( std::forward< std::vector< Primitive > >( primitives ) )
	{}

	std::shared_ptr< ModelInstance > Model::createInstance()
	{
		ZoneScoped;

		auto& buffers { getModelBuffers() };

		std::vector< PrimitiveInstanceInfoIndex > primitive_instances {};

		WorldTransform transform {};
		transform.translation = Coordinate< CoordinateSpace::World >( glm::vec3( 0.0, 0.0, 0.0 ) );
		transform.scale = glm::vec3( 0.007 );
		transform.rotation = { glm::quat( 1.0, 0.0, 0.0, 0.0 ) };

		const ModelInstanceInfo model_info { transform.mat4() };

		ModelInstanceInfoIndex model_instance { buffers.m_model_instances.acquire( model_info ) };

		for ( std::size_t i = 0; i < m_primitives.size(); i++ )
		{
			auto render_info { m_primitives[ i ].renderInstanceInfo() };

			PrimitiveInstanceInfo instance_info {};
			instance_info.m_primitive_info = render_info->idx();
			instance_info.m_model_info = model_instance.idx();
			instance_info.m_material = m_primitives[ i ].default_material->getID();

			primitive_instances.emplace_back( buffers.m_primitive_instances.acquire( instance_info ) );
		}

		return std::make_shared<
			ModelInstance >( std::move( primitive_instances ), std::move( model_instance ), this->shared_from_this() );
	}

	std::shared_ptr< Model > Model::createModel( const std::filesystem::path& path )
	{
		ZoneScoped;

		auto& buffers { getModelBuffers() };

		log::debug( "Creating model {}", path );

		ModelBuilder builder { buffers.m_vertex_buffer, buffers.m_index_buffer };
		builder.loadModel( path );

		//Calculate bounding box
		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_shared< Model >( std::move( builder.m_primitives ) ) };

		log::debug( "Finished creating model {}", path );

		return model_ptr;
	}

	std::shared_ptr< Model > Model::
		createModelFromVerts( std::vector< ModelVertex > verts, std::vector< std::uint32_t > indicies )
	{
		ZoneScoped;

		auto& buffers { getModelBuffers() };

		ModelBuilder builder { buffers.m_vertex_buffer, buffers.m_index_buffer };
		builder.loadVerts( std::move( verts ), std::move( indicies ) );

		OrientedBoundingBox bounding_box { buildBoundingBox( builder.m_primitives ) };

		auto model_ptr { std::make_shared< Model >( std::move( builder.m_primitives ) ) };

		return model_ptr;
	}

} // namespace fgl::engine