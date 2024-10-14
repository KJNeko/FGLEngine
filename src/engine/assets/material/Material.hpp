//
// Created by kj16609 on 10/12/24.
//

#pragma once

#include <memory>

#include "engine/types.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include "engine/constants.hpp"
#include "engine/descriptors/Descriptor.hpp"
#include "engine/descriptors/DescriptorSetLayout.hpp"
#include "engine/memory/buffers/vector/DeviceVector.hpp"
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#pragma GCC diagnostic pop

namespace fgl::engine
{
	namespace descriptors
	{
		class DescriptorSet;
	}
	class Texture;

	struct DeviceMaterialData;

	constexpr std::size_t MAX_MATERIAL_COUNT { 512 };

	constexpr descriptors::Descriptor material_descriptor { 0,
		                                                    vk::DescriptorType::eUniformBuffer,
		                                                    vk::ShaderStageFlagBits::eFragment,
		                                                    MAX_MATERIAL_COUNT,
		                                                    vk::DescriptorBindingFlagBits::eUpdateAfterBind
		                                                        | vk::DescriptorBindingFlagBits::ePartiallyBound };

	constexpr std::uint16_t MATERIAL_SET_ID { 3 };

	struct MaterialProperties
	{
		struct
		{
			std::shared_ptr< Texture > color_tex;
			glm::vec4 color_factors { 0.0f };

			std::shared_ptr< Texture > metallic_roughness_tex;
			float metallic_factor { 0.0f };
			float roughness_factor { 0.0f };
		} pbr;

		struct
		{
			float scale { 0.0f };
			std::shared_ptr< Texture > texture;
		} normal;

		struct
		{
			float strength { 0.0f };
			std::shared_ptr< Texture > texture;
		} occlusion;

		struct
		{
			glm::vec3 factors { 0.0f };
			std::shared_ptr< Texture > texture;
		} emissive;

		void writeData( DeviceMaterialData& data ) const;
		DeviceMaterialData data() const;
	};

	using MaterialID = std::uint32_t;

	//! Material data to be sent to the device
	// Alignas to prevent the struct from becoming bigger then needed
	struct DeviceMaterialData
	{
		TextureID color_texture_id { constants::INVALID_TEXTURE_ID };
		glm::vec4 color_factors { 0.0f, 0.0f, 0.0f, 0.0f };

		// Padding to shove metallic_texture_id to offset 32
		std::byte padd1[ 12 ];

		TextureID metallic_texture_id { constants::INVALID_TEXTURE_ID };
		float metallic_factor { 0.0f };
		float roughness_factor { 0.0f };

		TextureID normal_texture_id { constants::INVALID_TEXTURE_ID };
		float normal_tex_scale { 0.0f };

		TextureID occlusion_texture_id { constants::INVALID_TEXTURE_ID };
		float occlusion_tex_strength { 0.0f };

		TextureID emissive_texture_id { constants::INVALID_TEXTURE_ID };
		glm::vec3 emissive_factors { 0.0f, 0.0f, 0.0f };
	};

	static_assert( sizeof( DeviceMaterialData ) == 76 );
	static_assert( offsetof( DeviceMaterialData, metallic_texture_id ) == 32 );
	static_assert( offsetof( DeviceMaterialData, normal_texture_id ) == 44 );
	static_assert( offsetof( DeviceMaterialData, occlusion_texture_id ) == 52 );
	static_assert( offsetof( DeviceMaterialData, emissive_texture_id ) == 60 );
	static_assert( offsetof( DeviceMaterialData, emissive_factors ) == 64 );

	class Material
	{
		MaterialID m_id;

		Material();

	  public:

		MaterialProperties properties {};

		bool ready() const;

		void update();

		MaterialID getID() const;

		static std::shared_ptr< Material > createMaterial();

		~Material();

		static descriptors::DescriptorSetLayout& getDescriptorLayout();
		static descriptors::DescriptorSet& getDescriptorSet();
	};

	DeviceVector< DeviceMaterialData >& getDeviceMaterialGPUData();
	void initMaterialDataVec( memory::Buffer& buffer );

} // namespace fgl::engine
