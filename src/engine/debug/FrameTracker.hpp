//
// Created by kj16609 on 5/18/25.
//
#pragma once
#include <memory>
#include <vector>

namespace fgl::engine
{
	class FrameTrackerMark;

	class FrameTracker
	{
		std::vector< std::weak_ptr< FrameTrackerMark > > m_marks {};

		friend class FrameTrackerMark;

	  public:

		void increment();
	};

	class FrameTrackerMark : public std::enable_shared_from_this< FrameTrackerMark >
	{
		std::size_t m_counter { 0 };
		inline static std::weak_ptr< FrameTracker > s_tracker {};
		std::shared_ptr< FrameTracker > m_tracker;

		FrameTrackerMark();

	  public:

		~FrameTrackerMark();

		static std::shared_ptr< FrameTrackerMark > create();

		static void incrementAll();

		friend class FrameTracker;
	};
} // namespace fgl::engine