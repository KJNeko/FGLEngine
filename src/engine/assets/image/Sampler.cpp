//
// Created by kj16609 on 1/21/24.
//

#include "Sampler.hpp"

#include <vulkan/vulkan.hpp>

#include "engine/debug/logging/logging.hpp"
#include "engine/rendering/devices/Device.hpp"

namespace fgl::engine
{

	vk::raii::Sampler createSampler(
		const vk::Filter min_filter,
		const vk::Filter mag_filter,
		const vk::SamplerMipmapMode mipmode,
		const vk::SamplerAddressMode sampler_wrap_u,
		const vk::SamplerAddressMode sampler_wrap_v,
		const vk::SamplerAddressMode sampler_wrap_w )
	{
		vk::SamplerCreateInfo info;

		info.magFilter = mag_filter;
		info.minFilter = min_filter;

		info.mipmapMode = mipmode;

		info.addressModeU = sampler_wrap_u;
		info.addressModeV = sampler_wrap_v;
		info.addressModeW = sampler_wrap_w;

		if ( info.addressModeU == vk::SamplerAddressMode::eClampToBorder
		     || info.addressModeV == vk::SamplerAddressMode::eClampToBorder
		     || info.addressModeW == vk::SamplerAddressMode::eClampToBorder )
		{
			info.borderColor = vk::BorderColor::eFloatOpaqueWhite;
		}

		info.minLod = -1000;
		info.maxLod = 1000;

		info.maxAnisotropy = 1.0f;

		return Device::getInstance()->createSampler( info );
	}

	Sampler::Sampler(
		const vk::Filter min_filter,
		const vk::Filter mag_filter,
		const vk::SamplerMipmapMode mipmap_mode,
		const vk::SamplerAddressMode sampler_wrap_u,
		const vk::SamplerAddressMode sampler_wrap_v,
		const vk::SamplerAddressMode sampler_wrap_w ) :
	  m_sampler( createSampler( min_filter, mag_filter, mipmap_mode, sampler_wrap_u, sampler_wrap_v, sampler_wrap_w ) )
	{}

	namespace gl
	{
		vk::Filter filterToVk( const int value )
		{
			switch ( value )
			{
				default:
					throw std::runtime_error( "Failed to translate fitler value from opengl to vulkan!" );
				case GL_NEAREST:
					return vk::Filter::eNearest;
				case GL_LINEAR:
					return vk::Filter::eLinear;
				case GL_LINEAR_MIPMAP_LINEAR:
					return vk::Filter::eLinear;
			}

			FGL_UNREACHABLE();
		}

		vk::SamplerAddressMode wrappingToVk( const int val )
		{
			switch ( val )
			{
				default:
					throw std::runtime_error( "Failed to translate wrapping filter to vk address mode" );
				case GL_REPEAT:
					return vk::SamplerAddressMode::eRepeat;
				case GL_MIRRORED_REPEAT:
					return vk::SamplerAddressMode::eMirroredRepeat;
#ifdef GL_CLAMP_TO_BORDER
				case GL_CLAMP_TO_BORDER:
					return vk::SamplerAddressMode::eClampToBorder;
#endif
#ifdef GL_CLAMP_TO_EDGE
				case GL_CLAMP_TO_EDGE:
					return vk::SamplerAddressMode::eClampToEdge;
#endif
			}
		};

	} // namespace gl

	/**
	 *
	 * @param mag_filter
	 * @param min_filter
	 * @param wraps x wrap
	 * @param wrapt y wrap
	 */
	Sampler::Sampler( const int min_filter, const int mag_filter, const int wraps, const int wrapt ) :
	  Sampler(
		  gl::filterToVk( min_filter ),
		  gl::filterToVk( mag_filter ),
		  vk::SamplerMipmapMode::eLinear,
		  gl::wrappingToVk( wraps ),
		  gl::wrappingToVk( wrapt ) )
	{}

	Sampler::Sampler( Sampler&& other ) noexcept : m_sampler( std::move( other.m_sampler ) )
	{
		other.m_sampler = VK_NULL_HANDLE;
	}

	Sampler& Sampler::operator=( Sampler&& other ) noexcept
	{
		m_sampler = std::move( other.m_sampler );
		other.m_sampler = VK_NULL_HANDLE;
		return *this;
	}

	void Sampler::setName( const std::string& str ) const
	{
		vk::DebugUtilsObjectNameInfoEXT info {};
		info.objectType = vk::ObjectType::eSampler;
		info.pObjectName = str.c_str();
		info.setObjectHandle( reinterpret_cast< std::uint64_t >( static_cast< VkSampler >( *m_sampler ) ) );

		Device::getInstance().setDebugUtilsObjectName( info );
	}

} // namespace fgl::engine
