//
// Created by kj16609 on 2/17/25.
//
#include "ShadowMap.hpp"

#include "assets/image/Image.hpp"
#include "assets/texture/Texture.hpp"

namespace fgl::engine::shadows
{

	std::shared_ptr< Image > getDepthImage( const vk::Extent2D extent )
	{
		constexpr auto format { vk::Format::eR16Unorm };
		constexpr vk::ImageUsageFlags usage_flags { vk::ImageUsageFlagBits::eSampled
			                                        | vk::ImageUsageFlagBits::eDepthStencilAttachment };
		constexpr auto inital_layout { vk::ImageLayout::eUndefined };
		constexpr auto final_layout { vk::ImageLayout::eDepthReadOnlyOptimal };

		return std::make_shared< Image >( extent, format, usage_flags, inital_layout, final_layout );
	}

	PerFrameArray< std::shared_ptr< Image > > createDepthImages( vk::Extent2D extent )
	{
		PerFrameArray< std::shared_ptr< Image > > array {};

		for ( std::size_t i = 0; i < array.size(); ++i ) array[ i ] = getDepthImage( extent );

		return array;
	}

	PerFrameArray< std::shared_ptr< Texture > > ShadowMap::createDepthTargets()
	{
		PerFrameArray< std::shared_ptr< Texture > > array {};

		Sampler default_sampler {};
		for ( std::size_t i = 0; i < array.size(); ++i )
			array[ i ] = std::make_shared< Texture >( m_image[ i ], std::move( default_sampler ) );

		return array;
	}

	void ShadowMap::renderForCamera( const Camera& camera )
	{
		// model -> world -> camera (identity) -> screen (shadow)
		// since the camera in this case is the shadow map we just need to convert the screen space to world space. So we can just use an identity matrix

		const Matrix< MatrixType::WorldToCamera > camera_matrix { m_transform.mat() };
		const Matrix< MatrixType::CameraToScreen > identity { 1.0f };

		const Matrix< MatrixType::WorldToScreen > matrix { camera_matrix * identity };
	}

	ShadowMap::ShadowMap( const vk::Extent2D extent ) :
	  m_image( createDepthImages( extent ) ),
	  m_target( createDepthTargets() )
	{}

} // namespace fgl::engine::shadows