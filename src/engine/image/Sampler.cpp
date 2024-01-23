//
// Created by kj16609 on 1/21/24.
//

#include "Sampler.hpp"

#include <vulkan/vulkan.hpp>

#include "engine/Device.hpp"

namespace fgl::engine
{

	Sampler::Sampler(
		vk::Filter min_filter,
		vk::Filter mag_filter,
		vk::SamplerMipmapMode mipmap_mode,
		vk::SamplerAddressMode sampler_mode ) :
	  valid( true )
	{
		vk::SamplerCreateInfo info;

		info.magFilter = mag_filter;
		info.minFilter = min_filter;

		info.mipmapMode = mipmap_mode;

		info.addressModeU = sampler_mode;
		info.addressModeV = sampler_mode;
		info.addressModeW = sampler_mode;

		info.minLod = -1000;
		info.maxLod = 1000;

		info.maxAnisotropy = 1.0f;

		if ( Device::getInstance().device().createSampler( &info, nullptr, &m_sampler ) != vk::Result::eSuccess )
		{
			throw std::runtime_error( "Failed to create sampler" );
		}
	}

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

	Sampler::~Sampler()
	{
		if ( valid ) Device::getInstance().device().destroySampler( m_sampler );
	}

} // namespace fgl::engine
