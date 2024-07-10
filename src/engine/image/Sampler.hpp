//
// Created by kj16609 on 1/21/24.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "engine/FGL_DEFINES.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine
{

	class Sampler
	{
		vk::raii::Sampler m_sampler { VK_NULL_HANDLE };

	  public:

		FGL_DELETE_COPY( Sampler )

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
			vk::SamplerAddressMode sampler_wrap_u,
			vk::SamplerAddressMode sampler_wrap_v,
			vk::SamplerAddressMode sampler_wrap_w );

		FGL_FORCE_INLINE_FLATTEN Sampler(
			vk::Filter min_filter,
			vk::Filter mag_filter,
			vk::SamplerMipmapMode mipmap_mode,
			vk::SamplerAddressMode sampler_wrap_u,
			vk::SamplerAddressMode sampler_wrap_v ) :
		  Sampler( min_filter, mag_filter, mipmap_mode, sampler_wrap_u, sampler_wrap_v, sampler_wrap_v )
		{}

		FGL_FORCE_INLINE_FLATTEN Sampler(
			vk::Filter min_filter,
			vk::Filter mag_filter,
			vk::SamplerMipmapMode mipmap_mode,
			vk::SamplerAddressMode sampler_wrap_u ) :
		  Sampler( min_filter, mag_filter, mipmap_mode, sampler_wrap_u, sampler_wrap_u, sampler_wrap_u )
		{}

		Sampler( int min_filter, int mag_filter, int wraps, int wrapt );

		VkSampler operator*() { return *m_sampler; }

		Sampler( Sampler&& other );
		Sampler& operator=( Sampler&& );

		~Sampler() {}

		vk::raii::Sampler& getVkSampler() { return m_sampler; }

		void setName( const std::string str );
	};

} // namespace fgl::engine