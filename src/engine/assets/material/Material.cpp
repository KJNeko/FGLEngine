//
// Created by kj16609 on 10/12/24.
//

#include "Material.hpp"

#include "EngineContext.hpp"
#include "engine/assets/texture/Texture.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
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
		data.color.color_texture_id = getTexID( pbr.color_tex );
		data.color.color_factors = pbr.color_factors;

		data.metallic.metallic_texture_id = getTexID( pbr.metallic_roughness_tex );
		data.metallic.roughness_factor = pbr.roughness_factor;
		data.metallic.metallic_factor = pbr.metallic_factor;

		// Normal
		data.normal.normal_texture_id = getTexID( normal.texture );
		data.normal.normal_tex_scale = normal.scale;

		// Occlusion
		data.occlusion.occlusion_texture_id = getTexID( occlusion.texture );
		data.occlusion.occlusion_tex_strength = occlusion.strength;

		// Emissive
		data.emissive.emissive_texture_id = getTexID( emissive.texture );
		data.emissive.emissive_factors = emissive.factors;
	}

	DeviceMaterialData MaterialProperties::data() const
	{
		DeviceMaterialData data {};

		writeData( data );

		return data;
	}

	Material::Material() : m_id( material_id_counter.getID() )
	{
		getDescriptorSet().bindArray(
			0,
			EngineContext::getInstance().getMaterialManager().getBufferSuballocation(),
			m_id,
			sizeof( DeviceMaterialData ) );
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
		log::debug( "Destroyed material {}", m_id );
		material_id_counter.markUnused( m_id );
	}

	void Material::update()
	{
		const auto data { properties.data() };
		EngineContext::getInstance().getMaterialManager().update( m_id, data );
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
			//set->bindUniformBuffer( 0, getDeviceMaterialGPUData() );
			//set->update();
			//set = std::make_unique< descriptors::DescriptorSet >( std::move( set_layout.value() ) );
			set->setName( "Material descriptor set" );

			return *set;
		}
	}

} // namespace fgl::engine
