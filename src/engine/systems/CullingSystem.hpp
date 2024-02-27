//
// Created by kj16609 on 2/23/24.
//

#pragma once

#include <thread>

#include "engine/FrameInfo.hpp"

namespace fgl::engine
{

	class CullingSystem
	{
		std::thread m_thread;

		std::optional< FrameInfo* > m_info { std::nullopt };
		std::stop_token m_stop_token {};

		void runner();

		//Semaphore to signal the thread to start
		std::binary_semaphore m_start_sem { 0 };

		std::binary_semaphore m_end_sem { 0 };

	  public:

		CullingSystem() : m_thread( &CullingSystem::runner, this ) {}

		void pass( FrameInfo& info );
		void startPass( FrameInfo& info );

		void wait();
	};

} // namespace fgl::engine
