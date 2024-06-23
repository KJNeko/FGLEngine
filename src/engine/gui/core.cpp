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

#include <imgui_internal.h>

#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/filesystem/FileBrowser.hpp"
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
			.Subpass = 2,

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
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport( 0, nullptr, ImGuiDockNodeFlags_PassthruCentralNode );

		ImGui::Begin( "Main" );
	}

	void endImGui( vk::raii::CommandBuffer& command_buffer )
	{
		ImGui::End();
		ImGui::Render();

		ImDrawData* data { ImGui::GetDrawData() };
		ImGui_ImplVulkan_RenderDrawData( data, *command_buffer );

		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
	}

	void drawRenderingOutputs( const FrameInfo& info )
	{
		ImGui::Begin( "RenderOutputs" );

		enum RenderingOutputSelection : std::uint_fast8_t
		{
			Composite = 0,
			Albedo = 1,
			Normal = 2,
			Position = 3
		};

		static const char* const options[] = { "Composite", "Albedo", "Normal", "Position" };
		static std::uint_fast8_t current { Composite };

		if ( ImGui::BeginCombo( "Rendering Output", options[ current ] ) )
		{
			constexpr float desired_size { 64 };
			//Calculate size
			const float ratio { info.swap_chain.extentAspectRatio() };

			// h = w/h

			float fh_size { desired_size };
			float fv_size { desired_size * ratio };

			// If height is larger then the size then we need to compute the width from the height max
			if ( fv_size > desired_size )
			{
				fv_size = desired_size;
				fh_size = fv_size / ratio;
			}

			std::uint32_t h_size { static_cast< std::uint32_t >( fh_size ) };
			std::uint32_t v_size { static_cast< std::uint32_t >( fv_size ) };

			//Composite
			if ( ImGui::Selectable( options[ Composite ], current == Composite ) )
			{
				log::debug( "Changing output to Compositite" );
				current = Composite;
			}

			info.swap_chain.g_buffer_albedo_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Albedo ], current == Albedo ) )
			{
				log::debug( "Changing output to Albedo" );
				current = Albedo;
			}

			info.swap_chain.g_buffer_normal_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Normal ], current == Normal ) )
			{
				log::debug( "Changing output to Normal" );
				current = Normal;
			}

			info.swap_chain.g_buffer_position_img->drawImGui( { v_size, h_size } );
			ImGui::SameLine();
			if ( ImGui::Selectable( options[ Position ], current == Position ) )
			{
				log::debug( "Changing output to Position" );
				current = Position;
			}

			ImGui::EndCombo();
		}

		switch ( current )
		{
			default:
				[[fallthrough]];
			case Composite:
				info.swap_chain.g_buffer_composite_img->drawImGui();
				break;
			case Albedo:
				info.swap_chain.g_buffer_albedo_img->drawImGui();
				break;
			case Normal:
				info.swap_chain.g_buffer_normal_img->drawImGui();
				break;
			case Position:
				info.swap_chain.g_buffer_position_img->drawImGui();
				break;
		}

		ImGui::End();
	}

	void prepareDock()
	{
		// Docks seem utterly broken.
		ImGuiID primary_id { ImGui::GetID( "WindowGroup" ) };

		ImGui::DockSpaceOverViewport( primary_id, ImGui::GetMainViewport() );

		ImGui::DockBuilderRemoveNode( primary_id );
		ImGui::DockBuilderAddNode( primary_id, ImGuiDockNodeFlags_DockSpace );

		const auto viewport { ImGui::GetMainViewport() };
		ImGui::DockBuilderSetNodeSize( primary_id, viewport->Size );

		ImGuiID right {};
		auto left { ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Left, 0.2f, nullptr, &right ) };
		ImGui::DockBuilderAddNode( right );
		auto center { ImGui::DockBuilderSplitNode( right, ImGuiDir_Left, 0.2f, nullptr, &right ) };

		ImGui::DockBuilderAddNode( center, ImGuiDockNodeFlags_DockSpace );
		ImGui::DockBuilderAddNode( left, ImGuiDockNodeFlags_DockSpace );
		ImGui::DockBuilderAddNode( right, ImGuiDockNodeFlags_DockSpace );

		ImGuiID left_up {};
		auto left_down { ImGui::DockBuilderSplitNode( primary_id, ImGuiDir_Down, 0.2f, nullptr, &left_up ) };

		ImGui::DockBuilderDockWindow( "Scene", left_up );
		ImGui::DockBuilderDockWindow( "Main", left_down );

		ImGui::DockBuilderDockWindow( "Entity info", right );

		ImGuiID up {};
		auto down { ImGui::DockBuilderSplitNode( center, ImGuiDir_Down, 0.2f, nullptr, &up ) };

		ImGui::DockBuilderDockWindow( "RenderOutputs", up );
		ImGui::DockBuilderDockWindow( "File Picker", down );

		ImGui::DockBuilderFinish( primary_id );
	}

	void drawMainGUI( FrameInfo& info )
	{
		ZoneScoped;
		TracyVkZone( info.tracy_ctx, *info.command_buffer, "ImGui Rendering" );
		beginImGui();

		// TODO: Maybe play with docks again some other time
		//static std::once_flag flag;
		//std::call_once( flag, prepareDock );

		drawRenderingOutputs( info );
		drawEntityGUI( info );
		drawEntityInfo( info );
		drawFilesystemGUI( info );

		endImGui( info.command_buffer );
	}

	static GameObject* selected_object { nullptr };

	void drawEntityGUI( FrameInfo& info )
	{
		ImGui::Begin( "Scene" );

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

		if ( selected_object ) selected_object->drawImGui();

		ImGui::End();
	}

	void drawFilesystemGUI( FrameInfo& info )
	{
		ImGui::Begin( "File Picker", nullptr, ImGuiWindowFlags_MenuBar );

		filesystem::FileBrowser::drawGui( info );

		ImGui::End();
	}

	void cleanupImGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace fgl::engine::gui
