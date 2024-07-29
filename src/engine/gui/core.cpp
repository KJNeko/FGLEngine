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

#include <imgui.h>
#pragma GCC diagnostic pop

#include <imgui_internal.h>

#include "dockspace.hpp"
#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/filesystem/FileBrowser.hpp"
#include "engine/model/Model.hpp"
#include "engine/rendering/Device.hpp"
#include "engine/rendering/Renderer.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine::gui
{

	void initGui( const Window& window, const Renderer& renderer )
	{
		ZoneScoped;
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		[[maybe_unused]] ImGuiIO& io { ImGui::GetIO() };
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		io.ConfigWindowsResizeFromEdges = true;

		ImGui::StyleColorsDark();

		Device& device { Device::getInstance() };

		ImGui_ImplGlfw_InitForVulkan( window.window(), true );
		ImGui_ImplVulkan_InitInfo init_info {
			.Instance = device.instance(),
			.PhysicalDevice = *device.phyDevice().handle(),
			.Device = *device,
			.QueueFamily = device.phyDevice().queueInfo().getIndex( vk::QueueFlagBits::eGraphics ),
			.Queue = *device.graphicsQueue(),
			.DescriptorPool = *DescriptorPool::getInstance().getPool(),
			.RenderPass = *renderer.getSwapChainRenderPass(),
			.MinImageCount = 2,
			.ImageCount = 2,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,

			.PipelineCache = VK_NULL_HANDLE,
			.Subpass = 0,

			.UseDynamicRendering = VK_FALSE,
			.PipelineRenderingCreateInfo = {},

			.Allocator = VK_NULL_HANDLE,
			.CheckVkResultFn = VK_NULL_HANDLE,
			.MinAllocationSize = 1024 * 1024
		};

		ImGui_ImplVulkan_Init( &init_info );
	}

	void beginImGui()
	{
		ZoneScoped;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin( "Main" );
	}

	void endImGui( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;
		ImGui::End();
		ImGui::Render();

		ImDrawData* data { ImGui::GetDrawData() };
		ImGui_ImplVulkan_RenderDrawData( data, *command_buffer );

		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
	}

	inline void prepareDock( ImGuiID& primary_id )
	{
		ImGui::DockBuilderRemoveNode( primary_id );
		ImGui::DockBuilderAddNode( primary_id, ImGuiDockNodeFlags_DockSpace | ImGuiDockNodeFlags_PassthruCentralNode );

		ImGui::DockBuilderSetNodeSize( primary_id, ImGui::GetMainViewport()->WorkSize );

		constexpr float left_bar_size { 0.2f };
		constexpr float right_bar_size { 0.2f };
		constexpr float bottom_bar_size { 0.4f };

		ImGuiID lb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Left, left_bar_size, nullptr, &primary_id )
		};

		ImGuiID rb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Right, right_bar_size, nullptr, &primary_id )
		};

		ImGuiID bb_node {
			ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Down, bottom_bar_size, nullptr, &primary_id )
		};

		ImGuiID mv_node { primary_id };
		//ImGuiID mv_node { ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Up, 1.0f - 0.3f, &primary_id, nullptr ) };

		ImGui::DockBuilderDockWindow( "Scene", lb_node );

		//ImGui::DockBuilderDockWindow( "Main", mv_node );

		ImGui::DockBuilderDockWindow( "Camera: 0", mv_node );

		ImGui::DockBuilderDockWindow( "Entity info", rb_node );

		ImGui::DockBuilderDockWindow( "File Picker", bb_node );

		ImGui::DockBuilderFinish( primary_id );
	}

	// ImGui DockBuilder is still very much not ready for use.
	// But let's try it anyways
	void drawDock()
	{
		// ImGui::PushStyleVar( ImGuiStyleVar_WindowPadding, ImVec2( 0.0f, 0.0f ) );

		// Docks seem utterly broken.
		ImGuiID primary_id {
			ImGui::DockSpaceOverViewport( ImGui::GetID( "MainWindowDockspace" ), ImGui::GetMainViewport() )
		};
		// +--------------------------------------------------------------------+
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |                      mv                          |        |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |   lb   |                                                  |   rb   |
		// |        |                                                  |        |
		// |        |                                                  |        |
		// |        |--------------------------------------------------|        |
		// |        |                      bb                          |        |
		// |        |                                                  |        |
		// +--------+---------------------------------------------------+-------+

		static std::once_flag flag;
		std::call_once( flag, prepareDock, primary_id );

		ImGui::DockBuilderSetNodePos( primary_id, ImVec2( 0.0f, 0.0f ) );
		ImGui::DockBuilderSetNodeSize( primary_id, ImGui::GetMainViewport()->WorkSize );

		// ImGui::PopStyleVar();
	}

	void drawMainGUI( FrameInfo& info )
	{
		ZoneScoped;
		beginImGui();

		// ImGui::ShowDemoWindow();

		drawDock();

		drawCameraOutputs( info );
		drawEntityGUI( info );
		drawEntityInfo( info );
		drawFilesystemGUI( info );

		endImGui( info.command_buffer );
	}

	static GameObject* selected_object { nullptr };

	void drawEntityGUI( FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( "Scene" );

		for ( OctTreeNodeLeaf* leaf : info.game_objects.getAllLeafs() )
		{
			for ( GameObject& entity : *leaf )
			{
				ImGui::PushID( entity.getId() );

				if ( ImGui::Selectable( entity.getName().c_str() ) )
				{
					selected_object = &entity;
				}

				ImGui::PopID();
			}
		}

		ImGui::End();
	}

	void drawEntityInfo( [[maybe_unused]] FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( "Entity info" );

		if ( selected_object ) selected_object->drawImGui();

		ImGui::End();
	}

	void drawFilesystemGUI( FrameInfo& info )
	{
		ZoneScoped;
		ImGui::Begin( "File Picker", nullptr, ImGuiWindowFlags_MenuBar );

		filesystem::FileBrowser::drawGui( info );

		ImGui::End();
	}

	void cleanupImGui()
	{
		ZoneScoped;
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace fgl::engine::gui
