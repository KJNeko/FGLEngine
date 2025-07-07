//
// Created by kj16609 on 10/12/24.
//

#include "Material.hpp"

#include "EngineContext.hpp"
#include "assets/stores.hpp"
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
		data.color.color_texture_id = getTexID( m_pbr.m_color_tex );
		data.color.color_factors = m_pbr.m_color_factors;

		data.metallic.metallic_texture_id = getTexID( m_pbr.m_metallic_roughness_tex );
		data.metallic.roughness_factor = m_pbr.m_roughness_factor;
		data.metallic.metallic_factor = m_pbr.m_metallic_factor;

		// Normal
		data.normal.normal_texture_id = getTexID( m_normal.m_texture );
		data.normal.normal_tex_scale = m_normal.m_scale;

		// Occlusion
		data.occlusion.occlusion_texture_id = getTexID( m_occlusion.m_texture );
		data.occlusion.occlusion_tex_strength = m_occlusion.m_strength;

		// Emissive
		data.emissive.emissive_texture_id = getTexID( m_emissive.m_texture );
		data.emissive.emissive_factors = m_emissive.m_factors;
	}

	DeviceMaterialData MaterialProperties::data() const
	{
		DeviceMaterialData data {};

		writeData( data );

		return data;
	}

	MaterialProperties::MaterialProperties()
	{
		Sampler sampler {};

		std::shared_ptr< Texture > texture { getTextureStore().load( "assets/invalid.png", std::move( sampler ) ) };

		//Prepare the texture into the global system
		Texture::getDescriptorSet().bindTexture( 0, texture );
		Texture::getDescriptorSet().update();

		if ( !this->m_pbr.m_color_tex ) this->m_pbr.m_color_tex = texture;
	}

	Material::Material() : m_id( material_id_counter.getID() )
	{
		auto& descriptor_set { getDescriptorSet() };
		descriptor_set.bindArray(
			0,
			EngineContext::getInstance().getMaterialManager().getBufferSuballocation(),
			m_id,
			sizeof( DeviceMaterialData ) );
		descriptor_set.update();
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

		return checkTex( properties.m_pbr.m_color_tex ) && checkTex( properties.m_pbr.m_metallic_roughness_tex )
		    && checkTex( properties.m_normal.m_texture ) && checkTex( properties.m_occlusion.m_texture )
		    && checkTex( properties.m_emissive.m_texture );
	}

	std::shared_ptr< Material > Material::createMaterial()
	{
		return std::shared_ptr< Material >( new Material() );
	}

	std::shared_ptr< Material > Material::createNullMaterial()
	{
		//TODO: Make this a special material that will be rendered properly inside of the editor to represent no material was set
		auto mat { std::shared_ptr< Material >( new Material() ) };

		mat->update();

		return mat;
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
