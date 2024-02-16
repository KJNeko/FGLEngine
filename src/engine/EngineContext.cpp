//
// Created by kj16609 on 11/27/23.
//

#include "EngineContext.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <tracy/TracyVulkan.hpp>

#include <array>
#include <chrono>
#include <thread>

#include "KeyboardMovementController.hpp"
#include "engine/Average.hpp"
#include "engine/buffers/UniqueFrameSuballocation.hpp"
#include "engine/descriptors/Descriptor.hpp"
#include "engine/descriptors/DescriptorPool.hpp"
#include "engine/literals/size.hpp"
#include "engine/pipeline/PipelineT.hpp"
#include "engine/systems/EntityRendererSystem.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wconversion"
#include "engine/debug/drawers.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"
#include "imgui/imgui_internal.h"
#pragma GCC diagnostic pop

namespace fgl::engine
{
	constexpr float MAX_DELTA_TIME { 0.5 };

	EngineContext::EngineContext()
	{
		using namespace fgl::literals::size_literals;
		initGlobalStagingBuffer( 256_MiB );
#if ENABLE_IMGUI
		initImGui();
#endif
		loadGameObjects();
	}

	static Average< float, 120 > rolling_ms_average;

	void EngineContext::run()
	{
		using namespace fgl::literals::size_literals;
		Buffer global_ubo_buffer { 512_KiB,
			                       vk::BufferUsageFlagBits::eUniformBuffer,
			                       vk::MemoryPropertyFlagBits::eHostVisible }; // 512 KB

		PerFrameSuballocation< HostSingleT< CameraInfo > > camera_info { global_ubo_buffer,
			                                                             SwapChain::MAX_FRAMES_IN_FLIGHT };

		PerFrameSuballocation< HostSingleT< PointLight > > point_lights { global_ubo_buffer,
			                                                              SwapChain::MAX_FRAMES_IN_FLIGHT };

		constexpr std::uint32_t matrix_default_size { 16_MiB };
		constexpr std::uint32_t draw_parameter_default_size { 16_MiB };

		std::vector< Buffer > matrix_info_buffers {};

		std::vector< Buffer > draw_parameter_buffers {};

		std::vector< DescriptorSet > global_descriptor_sets {};

		for ( int i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			matrix_info_buffers.emplace_back(
				matrix_default_size,
				vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible );

			draw_parameter_buffers.emplace_back(
				draw_parameter_default_size,
				vk::BufferUsageFlagBits::eIndirectBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal | vk::MemoryPropertyFlagBits::eHostVisible );

			global_descriptor_sets.emplace_back( GlobalDescriptorSet::createLayout() );
		}

		for ( std::uint8_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; ++i )
		{
			global_descriptor_sets[ i ].setMaxIDX( 2 );
			global_descriptor_sets[ i ].bindUniformBuffer( 0, camera_info[ i ] );
			global_descriptor_sets[ i ].bindUniformBuffer( 2, point_lights[ i ] );
			global_descriptor_sets[ i ].update();
		}

		Camera camera {};

		auto viewer { GameObject::createGameObject() };
		viewer.transform.translation = constants::WORLD_CENTER + glm::vec3( 0.0f, 0.0f, -2.5f );
		KeyboardMovementController camera_controller {};

		auto current_time { std::chrono::high_resolution_clock::now() };

		auto previous_frame_start { std::chrono::high_resolution_clock::now() };

		//camera.setOrthographicProjection( -aspect, aspect, -1, 1, -1, 1 );
		const float aspect { m_renderer.getAspectRatio() };
		camera.setPerspectiveProjection( glm::radians( 90.0f ), aspect, constants::NEAR_PLANE, constants::FAR_PLANE );

		const auto old_aspect_ratio { m_renderer.getAspectRatio() };

		while ( !m_window.shouldClose() )
		{
			ZoneScopedN( "Poll" );
			glfwPollEvents();

			const auto new_time { std::chrono::high_resolution_clock::now() };

			{
				//Calculate time change from previous frame and add to accumulator
				const auto time_diff { new_time - previous_frame_start };
				rolling_ms_average.push(
					static_cast< float >( std::chrono::duration_cast< std::chrono::microseconds >( time_diff ).count() )
					/ 1000.0f );
				previous_frame_start = new_time;
			}

			auto delta_time {
				std::chrono::duration< float, std::chrono::seconds::period >( new_time - current_time ).count()
			};

			current_time = new_time;
			delta_time = glm::min( delta_time, MAX_DELTA_TIME );

#if ENABLE_IMGUI
			{
				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();
			}
#endif

			if ( old_aspect_ratio != m_renderer.getAspectRatio() )
			{
				camera.setPerspectiveProjection(
					glm::radians( 90.0f ), m_renderer.getAspectRatio(), constants::NEAR_PLANE, constants::FAR_PLANE );
			}

			camera_controller.moveInPlaneXZ( m_window.window(), delta_time, viewer );
			camera.setViewYXZ( viewer.transform.translation, viewer.transform.rotation );

			{
				constexpr WorldCoordinate center { 0.0f, 0.0f, 0.0f };
				//debug::world::drawVector( center, camera.getForward(), camera, { 0.0f, 0.0f, 0.0f } );

				{
					debug::world::drawFrustum( camera );
				}
			}

			if ( auto command_buffer = m_renderer.beginFrame(); command_buffer )
			{
				ZoneScopedN( "Render" );
				//Update
				const std::uint16_t frame_index { m_renderer.getFrameIndex() };

				const auto view_frustum { camera.getFrustumBounds() };

				FrameInfo frame_info { frame_index,
					                   delta_time,
					                   command_buffer,
					                   camera,
					                   global_descriptor_sets[ frame_index ],
					                   game_objects,
					                   m_renderer.getCurrentTracyCTX(),
					                   matrix_info_buffers[ frame_index ],
					                   draw_parameter_buffers[ frame_index ],
					                   m_renderer.getGBufferDescriptor( frame_index ),
					                   view_frustum };

#if TRACY_ENABLE
				auto& tracy_ctx { frame_info.tracy_ctx };
#endif

				CameraInfo current_camera_info { .projection = camera.getProjectionMatrix(),
					                             .view = camera.getViewMatrix() };

				camera_info[ frame_index ] = current_camera_info;

#if ENABLE_IMGUI
				{
					ZoneScopedN( "ImGui recording" );

					ImGui::Begin( "Titor Dev Menu" );

					ImGui::Text( "Framerate" );
					ImGui::SameLine();
					ImGui::Text( "%.1f FPS", ImGui::GetIO().Framerate );

					ImGui::Text( "Frame Time" );
					ImGui::SameLine();
					ImGui::Text( "%.3f ms", 1000.0f / ImGui::GetIO().Framerate );
					ImGui::Text( "Average rolling frametime: %.3f ms", rolling_ms_average.average() );

					auto inputVec3 = []( const std::string label, glm::vec3& vec )
					{
						ImGui::PushID( label.c_str() );
						ImGui::PushItemWidth( 80 );
						ImGui::Text( label.c_str() );
						ImGui::DragFloat( "X", &vec.x, 0.1f );
						ImGui::SameLine();
						ImGui::DragFloat( "Y", &vec.y, 0.1f );
						ImGui::SameLine();
						ImGui::DragFloat( "Z", &vec.z, 0.1f );
						ImGui::PopItemWidth();
						ImGui::PopID();
					};

					if ( ImGui::CollapsingHeader( "Camera" ) )
					{
						ImGui::PushItemWidth( 80 );
						ImGui::DragFloat( "Pos X", &viewer.transform.translation.x, 0.1f );
						ImGui::SameLine();
						ImGui::DragFloat( "Pos Y", &viewer.transform.translation.y, 0.1f );
						ImGui::SameLine();
						ImGui::DragFloat( "Pos Z", &viewer.transform.translation.z, 0.1f );
						ImGui::PopItemWidth();

						ImGui::Separator();

						ImGui::PushItemWidth( 80 );
						ImGui::DragFloat( "Rot X", &viewer.transform.rotation.x, 0.1f, 0.0f, glm::two_pi< float >() );
						ImGui::SameLine();
						ImGui::DragFloat( "Rot Y", &viewer.transform.rotation.y, 0.1f, 0.0f, glm::two_pi< float >() );
						ImGui::SameLine();
						ImGui::DragFloat( "Rot Z", &viewer.transform.rotation.z, 0.1f, 0.0f, glm::two_pi< float >() );
						ImGui::PopItemWidth();

						ImGui::Separator();
						ImGui::Checkbox( "Update Frustum", &camera.update_frustums );

						ImGui::Separator();
						ImGui::Checkbox( "Use Alt Frustum matrix", &camera.update_using_alt );
						if ( ImGui::CollapsingHeader( "Frustum matrix", &camera.update_using_alt ) )
						{
							ImGui::PushID( "FrustumMatrix" );
							inputVec3( "Translation", Camera::frustum_alt_transform.translation );
							inputVec3( "Rotation", Camera::frustum_alt_transform.rotation );
							ImGui::PopID();
						}
					}

					if ( ImGui::CollapsingHeader( "View Frustum" ) )
					{
						const auto& frustum { camera.getFrustumBounds() };

						auto printVec3 = []( const glm::vec3& vec )
						{ ImGui::Text( "(%.2f, %.2f, %.2f)", vec.x, vec.y, vec.z ); };

						auto printPlane = [ printVec3 ]( const auto& plane, const std::string name )
						{
							const std::string name_str { "Plane " + name + ": " };

							ImGui::Text( name_str.c_str() );
							ImGui::SameLine( 120.0f );
							printVec3( plane.direction() );
							ImGui::SameLine();
							ImGui::Text( "Distance: %.3f", plane.distance() );
							const auto pos { plane.getPosition() };
							ImGui::Text( "Center: %.2f %2.f %2.f", pos.x, pos.y, pos.z );
						};

						printPlane( frustum.near, "Near" );
						printPlane( frustum.far, "Far" );
						printPlane( frustum.top, "Top" );
						printPlane( frustum.bottom, "Bottom" );
						printPlane( frustum.right, "Right" );
						printPlane( frustum.left, "Left" );
					}

					if ( ImGui::CollapsingHeader( "Models" ) )
					{
						for ( auto& [ id, game_object ] : game_objects )
						{
							if ( game_object.model == nullptr ) continue;

							ImGui::PushID( std::to_string( id ).c_str() );

							if ( ImGui::TreeNode( game_object.model->getName().c_str() ) )
							{
								ImGui::PushID( game_object.model->getName().c_str() );
								{
									ImGui::PushID( "Position" );
									ImGui::PushItemWidth( 80 );
									ImGui::Text( "Position" );
									ImGui::SameLine();
									ImGui::DragFloat( "X", &game_object.transform.translation.x, 0.1f );
									ImGui::SameLine();
									ImGui::DragFloat( "Y", &game_object.transform.translation.y, 0.1f );
									ImGui::SameLine();
									ImGui::DragFloat( "Z", &game_object.transform.translation.z, 0.1f );
									ImGui::PopID();
								}

								ImGui::Separator();

								{
									ImGui::PushID( "Rotation" );
									ImGui::PushItemWidth( 80 );
									ImGui::Text( "Rotation" );
									ImGui::SameLine();
									ImGui::DragFloat(
										"X", &game_object.transform.rotation.x, 0.1f, 0.0f, glm::two_pi< float >() );
									ImGui::SameLine();
									ImGui::DragFloat(
										"Y", &game_object.transform.rotation.y, 0.1f, 0.0f, glm::two_pi< float >() );
									ImGui::SameLine();
									ImGui::DragFloat(
										"Z", &game_object.transform.rotation.z, 0.1f, 0.0f, glm::two_pi< float >() );
									ImGui::PopID();
								}

								ImGui::Separator();

								{
									ImGui::PushID( "Scale" );
									ImGui::PushItemWidth( 80 );
									ImGui::Text( "Scale" );
									ImGui::SameLine();
									ImGui::DragFloat( "X", &game_object.transform.scale.x, 0.1f );
									ImGui::SameLine();
									ImGui::DragFloat( "Y", &game_object.transform.scale.y, 0.1f );
									ImGui::SameLine();
									ImGui::DragFloat( "Z", &game_object.transform.scale.z, 0.1f );
									ImGui::TreePop();
									ImGui::PopID();
								}

								if ( ImGui::CollapsingHeader( "Textures" ) )
								{
									std::vector< TextureID > textures;

									ImGui::PushID( "Textures" );

									for ( auto& primitive : game_object.model->m_primitives )
									{
										if ( !primitive.m_texture.has_value() ) continue;

										auto& texture { primitive.m_texture.value() };

										const auto& extent { texture.getExtent() };

										auto& image_view { texture.getImageView() };
										auto& sampler { image_view.getSampler() };

										if ( !sampler.has_value() ) continue;

										ImVec2 size;
										size.x = extent.width;
										size.y = extent.height;

										if ( std::find( textures.begin(), textures.end(), texture.getID() )
										     == textures.end() )
										{
											textures.emplace_back( texture.getID() );

											ImGui::Image(
												static_cast< ImTextureID >( primitive.m_texture
											                                    ->getImGuiDescriptorSet() ),
												size );
										}
									}

									ImGui::PopID();
								}

								ImGui::PopID();
							}
							ImGui::PopID();
						}
					}

					//TODO: Add in a collapsable header to view all buffers, And their suballocations
					/*
					if ( ImGui::CollapsingHeader( "Buffer allocations" ) )
					{
						for ( const auto& buffer : Buffer::getActiveBufferHandles() )
						{
							ImGui::Text( "Address: %p", buffer.lock()->address() );
							ImGui::Text( "Size: %zu", buffer.lock()->size() );
						}
					}*/
				}
#endif

				m_renderer.beginSwapchainRendererPass( command_buffer );

				m_entity_renderer.pass( frame_info );

#if ENABLE_IMGUI
				{
#if TRACY_ENABLE
					TracyVkZone( tracy_ctx, command_buffer, "ImGui Rendering" );
#endif

					debug::world::drawPointText(
						Coordinate< CoordinateSpace::World >( 0.0f, 0.0f, 0.0f ), camera, { 1.0f, 0.0f, 0.0f } );

					ImGui::End();
					ImGui::Render();

					ImDrawData* data { ImGui::GetDrawData() };
					ImGui_ImplVulkan_RenderDrawData( data, command_buffer );
				}
#endif

				m_renderer.endSwapchainRendererPass( command_buffer );

#if TRACY_ENABLE
				TracyVkCollect( frame_info.tracy_ctx, command_buffer );
#endif

				m_renderer.endFrame();

				std::this_thread::sleep_until( new_time + std::chrono::milliseconds( 16 ) );

				FrameMark;
			}
		}

		Device::getInstance().device().waitIdle();
	}

	void EngineContext::loadGameObjects()
	{
		auto command_buffer { Device::getInstance().beginSingleTimeCommands() };

		/*
		{
			std::shared_ptr< Model > model { Model::createModel(
				Device::getInstance(),
				"models/night_heron8.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			for ( int i = 0; i < 16; ++i )
			{
				auto night_heron = GameObject::createGameObject();
				night_heron.model = model;
				night_heron.transform.translation = { -5.0f + ( i * 1.0 ), 0.5f, 0.0f };
				night_heron.transform.scale = { 0.05f, 0.05f, 0.05f };
				night_heron.transform.rotation = { 0.0f, 0.0f, 0.0f };

				night_heron.model->syncBuffers( command_buffer );

				game_objects.emplace( night_heron.getId(), std::move( night_heron ) );
			}
		}*/

		std::shared_ptr< Model > model { Model::createModel(
			Device::getInstance(),
			"models/khronos-sponza/Sponza.gltf",
			m_entity_renderer.getVertexBuffer(),
			m_entity_renderer.getIndexBuffer() ) };

		for ( int x = 0; x < 1; ++x )
		{
			for ( int y = 0; y < 1; ++y )
			{
				auto sponza = GameObject::createGameObject();
				sponza.model = model;
				sponza.transform.translation = { 0.0f + ( y * 30 ), 1.0f, 0.0f + ( x * 20 ) };
				sponza.transform.scale = { 0.007f, 0.007f, 0.007f };
				sponza.transform.rotation = { 0.0f, 0.0f, 0.0f };

				sponza.model->syncBuffers( command_buffer );

				game_objects.emplace( sponza.getId(), std::move( sponza ) );
			}
		}

		/*
		{
			std::shared_ptr< Model > model { Model::createModel(
				Device::getInstance(),
				"models/smooth_vase.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto smooth_vase = GameObject::createGameObject();
			smooth_vase.model = model;

			smooth_vase.transform.translation = { -0.5f, 0.5f, .0f };
			smooth_vase.transform.scale = { 3.0f, 1.5f, 3.0f };

			smooth_vase.model->syncBuffers( command_buffer );

			game_objects.emplace( smooth_vase.getId(), std::move( smooth_vase ) );
		}

		{
			std::shared_ptr< Model > flat_model { Model::createModel(
				Device::getInstance(),
				"models/flat_vase.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto flat_vase = GameObject::createGameObject();
			flat_vase.model = flat_model;
			//flat_vase.transform.translation = { 0.5f, 0.5f, 0.0f };
			flat_vase.transform.translation = { 0.5f, 0.5f, 0.0f };
			flat_vase.transform.scale = { 3.0f, 1.5f, 3.0f };

			flat_vase.model->syncBuffers( command_buffer );

			game_objects.emplace( flat_vase.getId(), std::move( flat_vase ) );
		}
		{
			std::shared_ptr< Model > quad_model { Model::createModel(
				Device::getInstance(),
				"models/quad.obj",
				m_entity_renderer.getVertexBuffer(),
				m_entity_renderer.getIndexBuffer() ) };

			auto floor = GameObject::createGameObject();
			floor.model = quad_model;
			floor.transform.translation = { 0.0f, 0.5f, 0.0f };
			floor.transform.scale = { 3.0f, 1.0f, 3.0f };
			floor.is_world = true;

			floor.model->syncBuffers( command_buffer );

			game_objects.emplace( floor.getId(), std::move( floor ) );
		}
		 */

		Device::getInstance().endSingleTimeCommands( command_buffer );
	}

	void EngineContext::initImGui()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io { ImGui::GetIO() };
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImGui::StyleColorsDark();

		auto& device { Device::getInstance() };

		ImGui_ImplGlfw_InitForVulkan( m_window.window(), true );
		ImGui_ImplVulkan_InitInfo init_info {
			.Instance = device.instance(),
			.PhysicalDevice = device.phyDevice(),
			.Device = device.device(),
			.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily,
			.Queue = device.graphicsQueue(),
			.PipelineCache = VK_NULL_HANDLE,
			.DescriptorPool = DescriptorPool::getInstance().getVkPool(),
			.Subpass = 1,
			.MinImageCount = 2,
			.ImageCount = 2,
			.MSAASamples = VK_SAMPLE_COUNT_1_BIT,
			.UseDynamicRendering = VK_FALSE,
			.ColorAttachmentFormat = {},
			.Allocator = VK_NULL_HANDLE,
			.CheckVkResultFn = VK_NULL_HANDLE,
		};

		ImGui_ImplVulkan_Init( &init_info, m_renderer.getSwapChainRenderPass() );
	}

	EngineContext::~EngineContext()
	{
#if ENABLE_IMGUI
		cleanupImGui();
#endif
	}

	void EngineContext::cleanupImGui()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

} // namespace fgl::engine