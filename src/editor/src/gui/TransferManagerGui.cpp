//
// Created by kj16609 on 5/16/25.
//

#include "core.hpp"
#include "engine/assets/transfer/TransferManager.hpp"
#include "memory/buffers/BufferSuballocationHandle.hpp"
#include "safe_include.hpp"

namespace fgl::engine::gui
{

	void drawTransferManager()
	{
		auto& transfer_manager { memory::TransferManager::getInstance() };

		ImGui::Text( "Queue size: " );
		ImGui::SameLine();
		ImGui::Text( "%ld", transfer_manager.m_queue.size() );

		ImGui::Text( "Processing: %ld", transfer_manager.m_processing.size() );

		ImGui::Text( "Staging buffer: " );
		ImGui::Indent();
		drawBufferInfo( *transfer_manager.m_staging_buffer.get() );
		ImGui::Unindent();

		ImGui::Separator();

		std::size_t i { 0 };
		for ( auto& transfer_data : transfer_manager.m_queue )
		{
			ImGui::Text( "Transfer #%zu:", i );
			ImGui::Indent();

			using TransferType = memory::TransferData::TransferType;

			const std::string_view type_str = []( const TransferType type ) -> std::string_view
			{
				switch ( type )
				{
					default:
						return "UNKNOWN";
					case TransferType::IMAGE_FROM_RAW:
						return "Image from raw";
					case TransferType::IMAGE_FROM_SUBALLOCATION:
						return "Image from suballocation";
					case TransferType::SUBALLOCATION_FROM_SUBALLOCATION:
						return "Suballocation from suballocation";
					case TransferType::SUBALLOCATION_FROM_RAW:
						return "Suballocation from raw";
				}
			}( transfer_data.m_type );

			ImGui::Text( "Type: %s", type_str.data() );
			ImGui::Text( "Ready: %s", transfer_data.isReady() ? "true" : "false" );
			ImGui::Text( "Needs staging: %s", transfer_data.needsStaging() ? "true" : "false" );

			auto printRaw = []( const memory::TransferData::RawData& raw_data )
			{ ImGui::Text( "Raw data size: %zu", raw_data.size() ); };

			auto printAllocation = []( const memory::TransferData::TransferSuballocationHandle& allocation )
			{
				ImGui::Text( "Buffer name: %s", allocation->getBuffer()->m_debug_name.c_str() );
				ImGui::Text( "Allocation size: %zu", allocation->bytesize() );
			};

			auto printImage = []( const memory::TransferData::TransferImageHandle& image )
			{ ImGui::Text( "Image: TODO" ); };

			auto visiter = [ &printRaw, &printAllocation, &printImage ]< typename T0 >( const T0& item )
			{
				using T = std::decay_t< T0 >;
				if constexpr ( std::is_same_v< T, memory::TransferData::RawData > )
					printRaw( item );
				else if constexpr ( std::is_same_v< T, memory::TransferData::TransferSuballocationHandle > )
					printAllocation( item );
				else if constexpr ( std::is_same_v< T, memory::TransferData::TransferImageHandle > )
					printImage( item );
			};

			ImGui::Text( "Source: " );
			ImGui::Indent();
			std::visit( visiter, transfer_data.source() );
			ImGui::Unindent();

			ImGui::Text( "Target: " );
			ImGui::Indent();
			std::visit( visiter, transfer_data.target() );
			ImGui::Unindent();

			ImGui::Unindent();
			ImGui::Separator();

			i += 1;
		}

		ImGui::Separator();
	}

} // namespace fgl::engine::gui
