//
// Created by kj16609 on 1/10/24.
//

#include "SuballocationView.hpp"

#include "BufferSuballocation.hpp"
#include "BufferSuballocationHandle.hpp"

namespace fgl::engine::memory
{

	vk::Buffer SuballocationView::getVkBuffer()
	{
		return m_suballocation->getVkBuffer();
	}

	vk::DeviceSize SuballocationView::offset()
	{
		return m_offset + m_suballocation->offset();
	}

	void SuballocationView::setOffset( vk::DeviceSize offset )
	{
		m_offset = offset;
	}

} // namespace fgl::engine::memory
