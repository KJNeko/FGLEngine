//
// Created by kj16609 on 4/20/25.
//
#include "ResourceWatcher.hpp"

#include "PresentSwapChain.hpp"
#include "globals.hpp"

namespace fgl::engine::watcher
{

	inline static PerFrameArray< ResourceWatcher > WATCHERS {};

	ResourceWatcher* get()
	{
		return &WATCHERS[ global::getCurrentFrameIndex() ];
	}

	void clearCurrent()
	{
		auto* ref { get() };
		std::lock_guard guard { ref->m_mutex };
		ref->m_resources.clear();
	}

} // namespace fgl::engine::watcher