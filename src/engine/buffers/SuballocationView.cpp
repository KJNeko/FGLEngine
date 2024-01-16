//
// Created by kj16609 on 1/10/24.
//

#include "SuballocationView.hpp"

#include "BufferSuballocation.hpp"
#include "BufferSuballocationHandle.hpp"

namespace fgl::engine
{

	vk::Buffer SuballocationView::getVkBuffer()
	{
		return m_suballocation->getVkBuffer();
	}

	vk::DeviceSize SuballocationView::getOffset()
	{
		return m_offset + m_suballocation->getOffset();
	}

	void SuballocationView::setOffset( vk::DeviceSize offset )
	{
		m_offset = offset;
	}

} // namespace fgl::engine
