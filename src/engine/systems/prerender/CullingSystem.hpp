//
// Created by kj16609 on 2/23/24.
//

#pragma once

#include <thread>

#include "engine/systems/concepts.hpp"
#include "rendering/pipelines/v2/Pipeline.hpp"

namespace fgl::engine
{
	struct FrameInfo;

	class CullingSystem
	{
		// std::thread m_thread;

		// std::optional< FrameInfo* > m_info { std::nullopt };
		// std::stop_source m_source {};
		// std::stop_token m_stop_token { m_source.get_token() };

		std::unique_ptr< Pipeline > m_cull_compute { nullptr };

		// void runner();

		// Semaphore to signal the thread to start
		// std::binary_semaphore m_start_sem { 0 };

		// std::binary_semaphore m_end_sem { 0 };

	  public:

		[[maybe_unused]] vk::raii::CommandBuffer& setupSystem( FrameInfo& info );

		CullingSystem();

		~CullingSystem();

		void pass( FrameInfo& info );
	};

	static_assert( is_system< CullingSystem > );
	// static_assert( is_threaded_system< CullingSystem > );

} // namespace fgl::engine
