//
// Created by kj16609 on 10/12/24.
//

#include "Material.hpp"

#include "engine/FrameInfo.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/memory/buffers/vector/DeviceVector.hpp"
#include "engine/utility/IDPool.hpp"

namespace fgl::engine
{

	inline static IDPool< MaterialID > material_id_counter { 1 };

	void MaterialProperties::writeData( DeviceMaterialData& data ) const
	{
		auto getTexID = []( const std::shared_ptr< Texture >& tex )
		{
			if ( tex ) return tex->getID();
			return constants::INVALID_TEXTURE_ID;
		};

		// PBR
		data.color_texture_id = getTexID( pbr.color_tex );
		data.color_factors = pbr.color_factors;

		data.metallic_texture_id = getTexID( pbr.metallic_roughness_tex );
		data.roughness_factor = pbr.roughness_factor;
		data.roughness_factor = pbr.roughness_factor;

		// Normal
		data.normal_texture_id = getTexID( normal.texture );
		data.normal_tex_scale = normal.scale;

		// Occlusion
		data.occlusion_texture_id = getTexID( occlusion.texture );
		data.occlusion_tex_strength = occlusion.strength;

		// Emissive
		data.emissive_texture_id = getTexID( emissive.texture );
		data.emissive_factors = emissive.factors;
	}

	DeviceMaterialData MaterialProperties::data() const
	{
		DeviceMaterialData data {};

		writeData( data );

		return data;
	}

	Material::Material() : m_id( material_id_counter.getID() )
	{
		update();
		getDescriptorSet().bindArray( 0, getDeviceMaterialGPUData().getHandle(), m_id, sizeof( DeviceMaterialData ) );
		getDescriptorSet().update();
	}

	bool Material::ready() const
	{
		auto checkTex = []( const std::shared_ptr< Texture >& tex )
		{
			if ( tex )
			{
				return tex->ready();
			}

			return true;
		};

		return checkTex( properties.pbr.color_tex ) && checkTex( properties.pbr.metallic_roughness_tex )
		    && checkTex( properties.normal.texture ) && checkTex( properties.occlusion.texture )
		    && checkTex( properties.emissive.texture );
	}

	std::shared_ptr< Material > Material::createMaterial()
	{
		return std::shared_ptr< Material >( new Material() );
	}

	Material::~Material()
	{
		material_id_counter.markUnused( m_id );
	}

	inline static std::unique_ptr< DeviceVector< DeviceMaterialData > > material_data {};

	void initMaterialDataVec( memory::Buffer& buffer )
	{
		material_data = std::make_unique< DeviceVector< DeviceMaterialData > >( buffer, MAX_MATERIAL_COUNT );
	}

	DeviceVector< DeviceMaterialData >& getDeviceMaterialGPUData()
	{
		if ( material_data )
			return *material_data;
		else
			throw std::runtime_error( "Material data gpu buffer not initalized!" );
	}

	void Material::update()
	{
		auto& data_vec { getDeviceMaterialGPUData() };

		data_vec.updateData( m_id, properties.data() );
	}

	MaterialID Material::getID() const
	{
		return m_id;
	}

	inline static descriptors::DescriptorSetLayout material_descriptor_set { MATERIAL_SET_ID, material_descriptor };

	descriptors::DescriptorSetLayout& Material::getDescriptorLayout()
	{
		return material_descriptor_set;
	}

	descriptors::DescriptorSet& Material::getDescriptorSet()
	{
		static std::unique_ptr< descriptors::DescriptorSet > set { nullptr };

		if ( set ) [[likely]]
			return *set;
		else
		{
			set = material_descriptor_set.create();
			assert( set->setIDX() == MATERIAL_SET_ID );
			set->setMaxIDX( 1 );
			//set->bindUniformBuffer( 0, getDeviceMaterialGPUData() );
			//set->update();
			//set = std::make_unique< descriptors::DescriptorSet >( std::move( set_layout.value() ) );
			set->setName( "Material descriptor set" );

			return *set;
		}
	}

} // namespace fgl::engine
