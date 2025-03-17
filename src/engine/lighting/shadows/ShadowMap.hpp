//
// Created by kj16609 on 2/17/25.
//
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include <memory>

#include "primitives/Transform.hpp"
#include "primitives/matricies/Matrix.hpp"
#include "primitives/matricies/MatrixEvolvedTypes.hpp"
#include "rendering/PresentSwapChain.hpp"

namespace fgl::engine
{
	class Image;
	class Texture;
} // namespace fgl::engine

namespace fgl::engine::shadows
{

	class ShadowMap
	{
		PerFrameArray< std::shared_ptr< Image > > m_image;
		PerFrameArray< std::shared_ptr< Texture > > m_target;

		Matrix< MatrixType::WorldToScreen > m_matrix { 1.0f };

		Transform< CoordinateSpace::World > m_transform {};

		PerFrameArray< std::shared_ptr< Texture > > createDepthTargets();

	  public:

		void renderForCamera( const Camera& camera );

		ShadowMap( vk::Extent2D extent );
		~ShadowMap();
	};

} // namespace fgl::engine::shadows

namespace fgl::engine
{
	using namespace shadows;
}