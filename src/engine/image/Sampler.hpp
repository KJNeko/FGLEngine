//
// Created by kj16609 on 1/21/24.
//

#pragma once

#include <vulkan/vulkan.hpp>

namespace fgl::engine
{

	class Sampler
	{
		bool valid { false };
		vk::Sampler m_sampler { VK_NULL_HANDLE };

	  public:

		Sampler() = delete;

		Sampler(
			vk::Filter min_filter,
			vk::Filter mag_filter,
			vk::SamplerMipmapMode mipmap_mode,
			vk::SamplerAddressMode sampler_mode );

		Sampler( const Sampler& ) = delete;
		Sampler& operator=( const Sampler& ) = delete;
		Sampler( Sampler&& other );
		Sampler& operator=( Sampler&& );

		~Sampler();

		vk::Sampler& getVkSampler() { return m_sampler; }
	};

} // namespace fgl::engine