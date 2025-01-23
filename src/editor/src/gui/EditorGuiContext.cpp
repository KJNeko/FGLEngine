//
// Created by kj16609 on 1/21/25.
//
#include "EditorGuiContext.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include <imgui_internal.h> // Included for DockBuilder since it's not exposed yet
#pragma GCC diagnostic pop

#include "FrameInfo.hpp"
#include "core.hpp"
#include "debug/timing/FlameGraph.hpp"
#include "descriptors/DescriptorPool.hpp"
#include "gui_window_names.hpp"
#include "rendering/RenderingFormats.hpp"

namespace fgl::editor
{

	using namespace fgl::engine;

	EditorGuiContext::EditorGuiContext( const Window& window )
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

		vk::PipelineRenderingCreateInfo pipeline_info {};

		//TODO: Remove the ability for pickPresentFormat to be used here. Instead this should be hidden from the end user in roder to prevent shittery. This should be applied to everything else here too within reason.
		const std::vector< vk::Format > color_formats { pickPresentFormat() };

		pipeline_info.setColorAttachmentFormats( color_formats );
		pipeline_info.setDepthAttachmentFormat( pickDepthFormat() );

		ImGui_ImplGlfw_InitForVulkan( window.window(), true );
		ImGui_ImplVulkan_InitInfo init_info {
			.Instance = device.instance(),
			.PhysicalDevice = *device.phyDevice().handle(),
			.Device = *device,
			.QueueFamily = device.phyDevice().queueInfo().getIndex( vk::QueueFlagBits::eGraphics ),
			.Queue = *device.graphicsQueue(),
			.DescriptorPool = *DescriptorPool::getInstance().getPool(),
			.RenderPass = VK_NULL_HANDLE,
			.MinImageCount = 2,
			.ImageCount = 2,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,

			.PipelineCache = VK_NULL_HANDLE,
			.Subpass = 0,

			.UseDynamicRendering = VK_TRUE,
			.PipelineRenderingCreateInfo = pipeline_info,

			.Allocator = VK_NULL_HANDLE,
			.CheckVkResultFn = VK_NULL_HANDLE,
			.MinAllocationSize = 1024 * 1024
		};

		ImGui_ImplVulkan_Init( &init_info );
	}

	EditorGuiContext::~EditorGuiContext()
	{
		ZoneScoped;

		log::info( "Destroying EditorGUIContext" );

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void EditorGuiContext::beginDraw()
	{
		ZoneScoped;
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void EditorGuiContext::draw( FrameInfo& info )
	{
		ZoneScoped;
		auto timer = debug::timing::push( "Draw ImGui" );
		ImGui::ShowDemoWindow();

		gui::drawDock();

		gui::drawCameraOutputs( info );
		gui::drawEntityGUI( info );
		gui::drawEntityInfo( info );

		ImGui::Begin( FILE_PICKER_NAME.data(), nullptr, ImGuiWindowFlags_MenuBar );

		m_file_browser.drawGui( info );

		ImGui::End();

		gui::drawStats( info );
	}

	void EditorGuiContext::endDraw( vk::raii::CommandBuffer& command_buffer )
	{
		ZoneScoped;
		ImGui::Render();

		ImDrawData* data { ImGui::GetDrawData() };
		ImGui_ImplVulkan_RenderDrawData( data, *command_buffer );

		//ImGui::UpdatePlatformWindows();
		//ImGui::RenderPlatformWindowsDefault();
	}

	void EditorGuiContext::endDraw( FrameInfo& info )
	{
		return endDraw( info.command_buffer );
	}
} // namespace fgl::editor