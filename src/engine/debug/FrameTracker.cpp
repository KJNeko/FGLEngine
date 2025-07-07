//
// Created by kj16609 on 5/18/25.
//
#include "FrameTracker.hpp"

namespace fgl::engine
{

	void FrameTracker::increment()
	{
		// remove any weak elements
		std::erase_if( m_marks, []( const auto& mark ) noexcept -> bool { return mark.expired(); } );

		// Lock and increment active elements
		for ( auto& mark : m_marks )
		{
			if ( const auto locked = mark.lock() )
			{
				locked->m_counter++;
			}
		}
	}

	FrameTrackerMark::FrameTrackerMark() :
	  m_tracker(
		  []() -> std::shared_ptr< FrameTracker >
		  {
			  if ( s_tracker.expired() )
			  {
				  auto new_tracker { std::make_shared< FrameTracker >() };
				  s_tracker = new_tracker;
				  return new_tracker;
			  }
			  return s_tracker.lock();
		  }() )
	{}

	FrameTrackerMark::~FrameTrackerMark() = default;

	std::shared_ptr< FrameTrackerMark > FrameTrackerMark::create()
	{
		auto ptr { std::shared_ptr< FrameTrackerMark >( new FrameTrackerMark() ) };
		ptr->m_tracker->m_marks.push_back( ptr );
		return ptr;
	}

	void FrameTrackerMark::incrementAll()
	{
		if ( auto tracker = s_tracker.lock() )
		{
			tracker->increment();
		}
	}
} // namespace fgl::engine
