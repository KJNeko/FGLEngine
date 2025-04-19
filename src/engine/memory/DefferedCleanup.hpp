//
// Created by kj16609 on 4/9/25.
//
#pragma once
#include "engine/globals.hpp"
#include "rendering/PresentSwapChain.hpp"
#include "rendering/types.hpp"

namespace fgl::engine::memory
{

	using Variant = std::variant<
		// Buffer,
		std::shared_ptr< BufferSuballocationHandle >,
		std::shared_ptr< descriptors::DescriptorSet >
		// Texture,
		// Image >;
		>;

	struct DefferedCleanup
	{
		inline static DefferedCleanup* m_instance {};

		PerFrameArray< std::vector< Variant > > m_to_cleanup {};

		DefferedCleanup();

		void cleanIdx( FrameIndex frame_index );
	};

	template < typename T >
	void deferredDelete( T& item )
	{
		FGL_ASSERT( DefferedCleanup::m_instance, "Deffered cleanup instance not ready!" );

		const auto frame_index { global::getCurrentFrameIndex() };

		( DefferedCleanup::m_instance->m_to_cleanup[ frame_index ] ).emplace_back( item );
	}

} // namespace fgl::engine::memory
