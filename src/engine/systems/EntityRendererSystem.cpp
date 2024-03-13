//
// Created by kj16609 on 11/27/23.
//

#include "EntityRendererSystem.hpp"

#include <engine/utils.hpp>
#include <imgui/imgui.h>
#include <tracy/TracyC.h>
#include <vulkan/vulkan.hpp>

#include <set>

#include "engine/debug/drawers.hpp"
#include "engine/literals/size.hpp"
#include "engine/tree/octtree/OctTreeNode.hpp"

namespace fgl::engine
{

	// <TextureID, MemoryOffset>
	using DrawKey = std::pair< TextureID, vk::DeviceSize >;
} // namespace fgl::engine

namespace std
{
	template <>
	struct hash< fgl::engine::DrawKey >
	{
		size_t operator()( const fgl::engine::DrawKey& key ) const
		{
			const auto id_hash { std::hash< fgl::engine::TextureID >()( key.first ) };
			const auto offset_hash { std::hash< vk::DeviceSize >()( key.second ) };

			size_t seed { 0 };
			fgl::engine::hashCombine( seed, id_hash, offset_hash );
			return seed;
		}
	};

} // namespace std

namespace fgl::engine
{
	std::unique_ptr< Buffer > m_global_draw_parameter_buffer { nullptr };

	void initDrawParameterBuffer( std::uint32_t size )
	{
		m_global_draw_parameter_buffer = std::make_unique< Buffer >(
			size,
			vk::BufferUsageFlagBits::eIndirectBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eDeviceLocal );
	}

	EntityRendererSystem::EntityRendererSystem( Device& device, VkRenderPass render_pass ) : m_device( device )
	{
		ZoneScoped;
		PipelineConfigInfo info { render_pass };

		for ( int i = 0; i < 4; ++i ) PipelineConfigInfo::addColorAttachmentConfig( info );

		info.subpass = 0;

		m_pipeline = std::make_unique< Pipeline >( m_device, info );

		using namespace fgl::literals::size_literals;

		initVertexBuffer( 128_MiB );
		initIndexBuffer( 64_MiB );
		initDrawParameterBuffer( 1_KiB );
	}

	using DrawPair = std::pair< vk::DrawIndexedIndirectCommand, std::vector< ModelMatrixInfo > >;

	bool operator<( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex < right.first.firstIndex;
	}

	bool operator==( const DrawPair& left, const DrawPair& right )
	{
		return left.first.firstIndex == right.first.firstIndex && left.first.indexCount && right.first.indexCount;
	}

	vk::CommandBuffer& EntityRendererSystem::setupSystem( FrameInfo& info )
	{
		auto& command_buffer { info.command_buffer };
		m_pipeline->bind( command_buffer );

		m_pipeline->bindDescriptor( command_buffer, 0, info.global_descriptor_set );
		m_pipeline->bindDescriptor( command_buffer, 1, Texture::getTextureDescriptorSet() );
		return command_buffer;
	}

	void EntityRendererSystem::pass( FrameInfo& info )
	{
		ZoneScopedN( "Entity pass" );
		auto& command_buffer { setupSystem( info ) };
		TracyVkZone( info.tracy_ctx, command_buffer, "Render game objects" );

		//TracyVkZone( m_device.getCurrentTracyCTX(), command_buffer, "Render game objects" );

		std::unordered_map< DrawKey, DrawPair > draw_pairs;

		std::uint64_t tri_counter { 0 };
		std::uint64_t object_counter { 0 };
		std::uint64_t primitive_counter { 0 };

		for ( auto* node : info.in_view_leafs )
		{
			ZoneScopedN( "Process leaf" );
			for ( const auto& obj : *node )
			{
				ZoneScopedN( "Process object" );
				if ( obj.m_model == nullptr ) continue;

				if ( !obj.m_is_visible ) continue;

				// debug::world::drawBoundingBox( obj.getBoundingBox() );

				++object_counter;

				for ( const auto& primitive : obj.m_model->m_primitives )
				{
					++primitive_counter;
					tri_counter += ( primitive.m_index_buffer.size() / 3 );

					const ModelMatrixInfo matrix_info { .model_matrix = obj.m_transform.mat4(),
						                                .texture_idx = primitive.m_texture->getID() };

					const auto key {
						std::make_pair( primitive.m_texture->getID(), primitive.m_index_buffer.getOffset() )
					};

					if ( auto itter = draw_pairs.find( key ); itter != draw_pairs.end() )
					{
						//Draw command for this mesh already exists. Simply add a count to it
						auto& [ itter_key, pair ] = *itter;
						auto& [ existing_cmd, model_matrix ] = pair;

						existing_cmd.instanceCount += 1;
						model_matrix.emplace_back( matrix_info );
						assert( model_matrix.size() == existing_cmd.instanceCount );
					}
					else
					{
						vk::DrawIndexedIndirectCommand cmd {};

						cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();
						cmd.indexCount = primitive.m_index_buffer.size();

						cmd.vertexOffset = static_cast< int32_t >( primitive.m_vertex_buffer.getOffsetCount() );

						cmd.instanceCount = 1;

						std::vector< ModelMatrixInfo > matrix_infos {};
						matrix_infos.reserve( 128 );
						matrix_infos.emplace_back( matrix_info );
						draw_pairs.emplace( key, std::make_pair( cmd, std::move( matrix_infos ) ) );
					}
				}
			}
		}

#if ENABLE_IMGUI
		ImGui::Text( "Tris: %lu", tri_counter );
		ImGui::Text( "Models: %lu", object_counter );
		ImGui::Text( "Primitives: %lu", primitive_counter );
#endif

		if ( draw_pairs.empty() )
		{
			std::cout << "Nothing to draw!" << std::endl;
			return;
		}

		std::vector< vk::DrawIndexedIndirectCommand > draw_commands {};
		std::vector< ModelMatrixInfo > model_matrices {};

		draw_commands.reserve( draw_pairs.size() );
		model_matrices.reserve( draw_pairs.size() * 2 );

		TracyCZoneN( filter_zone_TRACY, "Reorganize draw commands", true );
		for ( auto& [ key, pair ] : draw_pairs )
		{
			auto cmd { pair.first };
			assert( cmd != vk::DrawIndexedIndirectCommand() );
			cmd.firstInstance = static_cast< std::uint32_t >( model_matrices.size() );

			assert( cmd.instanceCount == pair.second.size() );

			assert( pair.second.size() > 0 );

			draw_commands.emplace_back( cmd );
			model_matrices.insert( model_matrices.end(), pair.second.begin(), pair.second.end() );
		}

		TracyCZoneEnd( filter_zone_TRACY );

		//Setup model matrix info buffers
		auto& model_matrix_info_buffer { m_model_matrix_info_buffers[ info.frame_idx ] };

		model_matrix_info_buffer =
			std::make_unique< ModelMatrixInfoBufferSuballocation >( info.model_matrix_info_buffer, model_matrices );

		model_matrix_info_buffer->flush();

		const auto& model_matricies_suballoc { model_matrix_info_buffer };

		assert( model_matrix_info_buffer->size() == model_matrices.size() );

		// Setup draw parameter buffer
		TracyCZoneN( draw_zone_TRACY, "Submit draw data", true );
		auto& draw_parameter_buffer { m_draw_parameter_buffers[ info.frame_idx ] };

		draw_parameter_buffer =
			std::make_unique< DrawParameterBufferSuballocation >( info.draw_parameter_buffer, draw_commands );

		const auto& draw_params { draw_parameter_buffer };
		assert( draw_params->size() == draw_commands.size() );
		assert( draw_params->stride() == sizeof( vk::DrawIndexedIndirectCommand ) );

		TracyCZoneEnd( draw_zone_TRACY );

		draw_parameter_buffer->flush();
		const std::vector< vk::Buffer > vertex_buffers { m_vertex_buffer->getVkBuffer(),
			                                             model_matricies_suballoc->getVkBuffer() };

		command_buffer.bindVertexBuffers( 0, vertex_buffers, { 0, model_matricies_suballoc->getOffset() } );
		command_buffer.bindIndexBuffer( m_index_buffer->getVkBuffer(), 0, vk::IndexType::eUint32 );

#if ENABLE_IMGUI
		ImGui::Text( "Indirect draws: %lu", static_cast< std::size_t >( draw_params->size() ) );
#endif

		command_buffer.drawIndexedIndirect(
			draw_params->getVkBuffer(), draw_params->getOffset(), draw_params->size(), draw_params->stride() );
	}

} // namespace fgl::engine
