//
// Created by kj16609 on 1/21/24.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

namespace fgl::engine
{

	class Sampler
	{
		bool valid;
		vk::raii::Sampler m_sampler;

	  public:

		Sampler() :
		  Sampler(
			  vk::Filter::eLinear,
			  vk::Filter::eLinear,
			  vk::SamplerMipmapMode::eLinear,
			  vk::SamplerAddressMode::eClampToBorder )
		{}

		Sampler(
			vk::Filter min_filter,
			vk::Filter mag_filter,
			vk::SamplerMipmapMode mipmap_mode,
			vk::SamplerAddressMode sampler_mode );

		VkSampler operator*() { return *m_sampler; }

		Sampler( const Sampler& ) = delete;
		Sampler& operator=( const Sampler& ) = delete;
		Sampler( Sampler&& other );
		Sampler& operator=( Sampler&& );

		vk::raii::Sampler& getVkSampler() { return m_sampler; }
	};

} // namespace fgl::engine