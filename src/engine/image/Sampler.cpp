//
// Created by kj16609 on 1/21/24.
//

#include "Sampler.hpp"

#include <vulkan/vulkan.hpp>

#include "engine/rendering/Device.hpp"

namespace fgl::engine
{

	vk::raii::Sampler createSampler(
		vk::Filter min_filter,
		vk::Filter mag_filter,
		vk::SamplerMipmapMode mipmode,
		vk::SamplerAddressMode address_mode )
	{
		vk::SamplerCreateInfo info;

		info.magFilter = mag_filter;
		info.minFilter = min_filter;

		info.mipmapMode = mipmode;

		info.addressModeU = address_mode;
		info.addressModeV = address_mode;
		info.addressModeW = address_mode;

		info.minLod = -1000;
		info.maxLod = 1000;

		info.maxAnisotropy = 1.0f;

		return Device::getInstance()->createSampler( info );
	}

	Sampler::Sampler(
		vk::Filter min_filter,
		vk::Filter mag_filter,
		vk::SamplerMipmapMode mipmap_mode,
		vk::SamplerAddressMode sampler_mode ) :
	  valid( true ),
	  m_sampler( createSampler( mag_filter, min_filter, mipmap_mode, sampler_mode ) )
	{}

	Sampler::Sampler( Sampler&& other ) : valid( other.valid ), m_sampler( std::move( other.m_sampler ) )
	{
		other.valid = false;
	}

	Sampler& Sampler::operator=( Sampler&& other )
	{
		this->valid = other.valid;
		other.valid = false;
		if ( valid ) m_sampler = std::move( other.m_sampler );
		return *this;
	}

} // namespace fgl::engine
