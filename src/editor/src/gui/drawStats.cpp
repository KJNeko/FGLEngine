
#include <vulkan/vulkan.hpp>

#include "core.hpp"
#include "engine/buffers/Buffer.hpp"
#include "safe_include.hpp"

namespace fgl::engine::gui
{

	struct AllocationList
	{
		struct AllocationInfo
		{
			vk::DeviceSize m_total;
			vk::DeviceSize m_used;

			inline vk::DeviceSize free() const { return m_total - m_used; }

			vk::DeviceSize m_largest_free_block;
		};

		AllocationInfo gpu {};
		AllocationInfo host {};
	};

	const AllocationList getTotalAllocated()
	{
		AllocationList info {};

		auto& [ gpu_allocated, gpu_used, gpu_largest_free ] = info.gpu;
		auto& [ host_allocated, host_used, host_largest_free ] = info.host;

		for ( const std::weak_ptr< memory::BufferHandle >& buffer_weak :
		      fgl::engine::memory::Buffer::getActiveBufferHandles() )
		{
			if ( auto locked = buffer_weak.lock(); locked )
			{
				// The buffer is still active.
				if ( locked->m_memory_properties & vk::MemoryPropertyFlagBits::eDeviceLocal )
				{
					gpu_allocated += locked->size();
				}
				else if ( locked->m_memory_properties & vk::MemoryPropertyFlagBits::eHostVisible )
				{
					host_allocated += locked->size();
				}
				else
					throw std::
						runtime_error( "Unknown memory property flag choice. Could not determine host vs device" );
			}
		}

		return info;
	}

	void drawMemoryStats()
	{
		const auto [ gpu, host ] = getTotalAllocated();
		const auto& [ gpu_allocated, gpu_used, gpu_largest_block ] = gpu;
		const auto& [ host_allocated, host_used, host_largest_block ] = host;

		ImGui::Text( "Device" );
		ImGui::Text( "|- %0.1f MB Allocated", static_cast< float >( gpu_allocated ) / 1000.0f / 1000.0f );
		ImGui::Text( "|- %0.1f MB Used ", static_cast< float >( gpu_used ) / 1000.0f / 1000.0f );
		ImGui::Text( "|- %0.1f MB Unused", static_cast< float >( gpu.free() ) / 1000.0f / 1000.0f );

		ImGui::Separator();
		ImGui::Text( "Host" );
		ImGui::Text( "|- %0.1f MB Allocated", static_cast< float >( host_allocated ) / 1000.0f / 1000.0f );
		ImGui::Text( "|- %0.1f MB Used ", static_cast< float >( host_used ) / 1000.0f / 1000.0f );
		ImGui::Text( "|- %0.1f MB Unused", static_cast< float >( host.free() ) / 1000.0f / 1000.0f );
		ImGui::Separator();
	}

	void drawStats( const FrameInfo& info )
	{
		ImGui::Begin( "Stats" );

		ImGui::Text( "FPS: %0.1f", ImGui::GetIO().Framerate );

		if ( ImGui::CollapsingHeader( "Memory" ) )
		{
			drawMemoryStats();
		}

		ImGui::End();
	}

} // namespace fgl::engine::gui
