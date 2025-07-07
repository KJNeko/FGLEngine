//
// Created by kj16609 on 2/14/25.
//
#pragma once
#include <vulkan/vulkan_raii.hpp>

#include "FGL_DEFINES.hpp"

namespace fgl::engine
{
	class CommandBufferPool;

	/**
	 * @brief Contains a vk::raii::CommandBuffer to be used for recording, and a list of all assets to be appended too for each asset used.
	 */
	class CommandBufferHandle
	{
	  public:

		enum CommandType : std::uint8_t
		{
			Invalid = 0,
			Primary = 1,
			Secondary = 2
		} m_type;

	  private:

		/*
		 * Later on we'll want to append to this list in order to keep various assets alive
		 * that need to persist for this command buffer. Once this command buffer is reset,
		 * we can then drop all the assets kept in this list if they are ready to be deleted
		 */
		// std::vector< std::shared_ptr< AssetHandle > > m_asset_handles {};

		vk::raii::CommandBuffer m_cmd_buffer;

		friend class CommandBufferPool;

		explicit CommandBufferHandle( vk::raii::CommandBuffer&& buffer, CommandType type );

	  public:

		FGL_DELETE_COPY( CommandBufferHandle );
		FGL_DELETE_MOVE( CommandBufferHandle );
		CommandBufferHandle() = delete;

		~CommandBufferHandle();

		vk::raii::CommandBuffer& cmd();
	};

} // namespace fgl::engine