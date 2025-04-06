//
// Created by kj16609 on 1/23/25.
//
#include "MaterialManager.hpp"

#include "engine/debug/logging/logging.hpp"
#include "engine/math/literals/size.hpp"
#include "material/Material.hpp"

namespace fgl::engine
{

	using namespace fgl::literals::size_literals;

	void MaterialManager::update( const std::size_t idx, const DeviceMaterialData& material_data )
	{
		m_material_data.updateData( idx, material_data );
	}

	MaterialManager::MaterialManager() :
	  m_material_data_pool(
		  1_MiB,
		  vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst,
		  vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible ),
	  m_material_data( m_material_data_pool, MAX_MATERIAL_COUNT )
	{
		m_material_data_pool.setDebugName( "Material data pool" );
	}

	MaterialManager::~MaterialManager()
	{
		for ( const auto& track : debug::getTracks( "GPU", "Material" ) )
		{
			log::info( "Found leftover material: \n{}", track.trace );
		}
	}

	memory::Buffer& MaterialManager::getBuffer()
	{
		return m_material_data_pool;
	}

	memory::BufferSuballocation& MaterialManager::getBufferSuballocation()
	{
		return m_material_data;
	}

	DeviceVector< DeviceMaterialData >& MaterialManager::getMaterialData()
	{
		return m_material_data;
	}

} // namespace fgl::engine
