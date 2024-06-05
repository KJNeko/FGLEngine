//
// Created by kj16609 on 6/3/24.
//

#include "core.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include <imgui.h>
#pragma GCC diagnostic pop

#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/model/Model.hpp"
#include "engine/rendering/Device.hpp"
#include "engine/rendering/Renderer.hpp"

namespace fgl::engine::gui
{

	void initGui( const Window& window, const Renderer& renderer )
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		[[maybe_unused]] ImGuiIO& io { ImGui::GetIO() };
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsResizeFromEdges = true;

		ImGui::StyleColorsDark();

		Device& device { Device::getInstance() };

		ImGui_ImplGlfw_InitForVulkan( window.window(), true );
		ImGui_ImplVulkan_InitInfo init_info { .Instance = device.instance(),
			                                  .PhysicalDevice = device.phyDevice(),
			                                  .Device = device.device(),
			                                  .QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily,
			                                  .Queue = device.graphicsQueue(),
			                                  .DescriptorPool = DescriptorPool::getInstance().getVkPool(),
			                                  .RenderPass = renderer.getSwapChainRenderPass(),
			                                  .MinImageCount = 2,
			                                  .ImageCount = 2,
			                                  .MSAASamples = VK_SAMPLE_COUNT_1_BIT,

			                                  .PipelineCache = VK_NULL_HANDLE,
			                                  .Subpass = 1,

			                                  .UseDynamicRendering = VK_FALSE,
			                                  .PipelineRenderingCreateInfo = {},

			                                  .Allocator = VK_NULL_HANDLE,
			                                  .CheckVkResultFn = VK_NULL_HANDLE,
			                                  .MinAllocationSize = 1024 * 1024 };

		ImGui_ImplVulkan_Init( &init_info );
	}

	void beginImGui()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport( 0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode );

		ImGui::Begin( "Main" );
	}

	void endImGui( vk::CommandBuffer& command_buffer )
	{
		ImGui::End();
		ImGui::Render();

		ImDrawData* data { ImGui::GetDrawData() };
		ImGui_ImplVulkan_RenderDrawData( data, command_buffer );

		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}

	void drawEntityInfo( FrameInfo& );

	void drawMainGUI( FrameInfo& info )
	{
		ZoneScoped;
		//TracyVkZone( tracy_ctx, command_buffer, "ImGui Rendering" );
		beginImGui();

		drawEntityGUI( info );
		drawEntityInfo( info );

		endImGui( info.command_buffer );
	}

	static GameObject* selected_object { nullptr };

	void drawEntityGUI( FrameInfo& info )
	{
		ImGui::Begin( "Entities" );

		if ( selected_object )
			ImGui::Text( "Selected Object: %s, %d", selected_object->name.c_str(), selected_object->m_id );
		else
			ImGui::Text( "Selected Object: None" );

		ImGui::Separator();

		for ( OctTreeNodeLeaf* leaf : info.game_objects.getAllLeafs() )
		{
			for ( GameObject& entity : *leaf )
			{
				ImGui::PushID( entity.m_id );

				if ( ImGui::Selectable( entity.name.c_str() ) )
				{
					selected_object = &entity;
				}

				ImGui::PopID();
			}
		}

		ImGui::End();
	}

	void drawEntityInfo( FrameInfo& info )
	{
		ImGui::Begin( "Entity info" );

		if ( !selected_object )
		{
			ImGui::End();
			return;
		}

		ImGui::InputText( "Name", &selected_object->name );

		ImGui::Text( "Model: " );

		ImGui::End();
	}

	void cleanupImGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace fgl::engine::gui
