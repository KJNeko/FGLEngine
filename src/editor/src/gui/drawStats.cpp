
#include <vulkan/vulkan.hpp>

#include "core.hpp"
#include "engine/debug/profiling/counters.hpp"
#include "engine/debug/timing/FlameGraph.hpp"
#include "engine/flags.hpp"
#include "engine/math/literals/size.hpp"
#include "engine/memory/buffers/Buffer.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"
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

			vk::DeviceSize m_largest_free_block { std::numeric_limits< vk::DeviceSize >::max() };
		};

		AllocationInfo gpu {};
		AllocationInfo host {};
	};

	AllocationList getTotalAllocated()
	{
		AllocationList info {};

		auto& [ gpu_allocated, gpu_used, gpu_largest_free ] = info.gpu;
		auto& [ host_allocated, host_used, host_largest_free ] = info.host;

		for ( const auto* buffer : memory::getActiveBuffers() )
		{
			// The buffer is still active.
			if ( buffer->m_memory_properties & vk::MemoryPropertyFlagBits::eDeviceLocal )
			{
				gpu_allocated += buffer->size();
				gpu_used += buffer->used();
				gpu_largest_free = std::min( buffer->largestBlock(), gpu_largest_free );
			}
			else if ( buffer->m_memory_properties & vk::MemoryPropertyFlagBits::eHostVisible )
			{
				host_allocated += buffer->size();
				host_used += buffer->used();
				host_largest_free = std::min( buffer->largestBlock(), host_largest_free );
			}
			else
				throw std::runtime_error( "Unknown memory property flag choice. Could not determine host vs device" );
		}

		return info;
	}

	void drawMemoryStats()
	{
		const auto [ gpu, host ] = getTotalAllocated();
		const auto& [ gpu_allocated, gpu_used, gpu_largest_block ] = gpu;
		const auto& [ host_allocated, host_used, host_largest_block ] = host;

		using namespace literals::size_literals;

		if ( ImGui::TreeNode( "Device" ) )
		{
			ImGui::Text( "|- %s Allocated", toString( gpu_allocated ).c_str() );
			ImGui::Text( "|- %s Used ", toString( gpu_used ).c_str() );
			ImGui::Text( "|- %s Unused", toString( gpu.free() ).c_str() );
			ImGui::Text( "|- %s Available in most allocated buffer", toString( gpu.m_largest_free_block ).c_str() );
			ImGui::TreePop();
		}

		ImGui::Separator();

		if ( ImGui::TreeNode( "Host" ) )
		{
			ImGui::Text( "|- %s Allocated", toString( host_allocated ).c_str() );
			ImGui::Text( "|- %s Used ", toString( host_used ).c_str() );
			ImGui::Text( "|- %s Unused", toString( host.free() ).c_str() );
			ImGui::Text( "|- %s Available in most allocated buffer", toString( host.m_largest_free_block ).c_str() );
			ImGui::TreePop();
		}
		ImGui::Separator();

		if ( ImGui::CollapsingHeader( "Buffers" ) )
		{
			for ( const auto* buffer : memory::getActiveBuffers() )
			{
				ImGui::Text( "Name: %s", buffer->m_debug_name.c_str() );
				ImGui::Text(
					"Allocated: %s/%s (%2.1f\%)",
					toString( buffer->used() ).c_str(),
					toString( buffer->size() ).c_str(),
					( static_cast< float >( buffer->used() ) / static_cast< float >( buffer->size() ) * 100.0f ) );
				ImGui::Text( "Largest block: %s", toString( buffer->largestBlock() ).c_str() );
				ImGui::Separator();
			}
		}
	}

	void drawStats( const FrameInfo& info )
	{
		ImGui::Begin( "Stats" );

		ImGui::Text( "FPS: %0.1f", ImGui::GetIO().Framerate );
		const auto& counters { profiling::getCounters() };
		ImGui::Text( "Models drawn: %zu", counters.m_models_draw );
		ImGui::Text( "Verts drawn: %zu", counters.m_verts_drawn );
		ImGui::Text( "Draw instances: %zu", counters.m_instance_count );

		if ( ImGui::CollapsingHeader( "Memory" ) )
		{
			drawMemoryStats();
		}

		if ( ImGui::CollapsingHeader( "Timings" ) )
		{
			debug::timing::render();
		}

		imGuiOctTreeSettings( info );

		if ( ImGui::Button( "Reload shaders" ) )
		{
			flags::triggerShaderReload();
		}

		ImGui::End();
	}

} // namespace fgl::engine::gui
