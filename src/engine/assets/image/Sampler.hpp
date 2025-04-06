//
// Created by kj16609 on 1/21/24.
//

#pragma once

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

#include "engine/FGL_DEFINES.hpp"

namespace fgl::engine
{

	class Sampler
	{
		vk::raii::Sampler m_sampler;

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
			const vk::Filter min_filter,
			const vk::Filter mag_filter,
			const vk::SamplerMipmapMode mipmap_mode,
			const vk::SamplerAddressMode sampler_wrap_u,
			const vk::SamplerAddressMode sampler_wrap_v ) :
		  Sampler( min_filter, mag_filter, mipmap_mode, sampler_wrap_u, sampler_wrap_v, sampler_wrap_v )
		{}

		FGL_FORCE_INLINE_FLATTEN Sampler(
			const vk::Filter min_filter,
			const vk::Filter mag_filter,
			const vk::SamplerMipmapMode mipmap_mode,
			const vk::SamplerAddressMode sampler_wrap_u ) :
		  Sampler( min_filter, mag_filter, mipmap_mode, sampler_wrap_u, sampler_wrap_u, sampler_wrap_u )
		{}

		//TODO: This should be moved to favor a single sampler when possible, Making a duplicate sampler is not required.
		//TODO: Singleton
		Sampler( int min_filter, int mag_filter, int wraps, int wrapt );

		VkSampler operator*() const { return *m_sampler; }

		Sampler( Sampler&& other ) noexcept;
		Sampler& operator=( Sampler&& ) noexcept;

		~Sampler() {}

		const vk::raii::Sampler& getVkSampler() const { return m_sampler; }

		void setName( const std::string& str ) const;
	};

} // namespace fgl::engine