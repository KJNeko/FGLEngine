//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <filesystem>
#include <fstream>

namespace fgl::engine
{

	struct Shader
	{
		std::filesystem::path m_path;
		std::vector< std::byte > shader_data;
		vk::ShaderModuleCreateInfo module_create_info;
		vk::PipelineShaderStageCreateInfo stage_info;

		vk::raii::ShaderModule shader_module;

		static std::vector< std::byte > loadData( const std::filesystem::path& );
		vk::ShaderModuleCreateInfo createModuleInfo() const;

		Shader( const std::filesystem::path& path, const vk::PipelineShaderStageCreateInfo& info );

		Shader( const Shader& other ) = delete;

		Shader& operator=( const Shader& other ) = delete;

		Shader( Shader&& other ) = delete;

		Shader& operator=( Shader&& other ) = delete;

		static std::shared_ptr< Shader > loadShader( std::filesystem::path path, vk::ShaderStageFlagBits stage_flags );

		inline static std::shared_ptr< Shader > loadVertex( std::filesystem::path path )
		{
			return loadShader( path, vk::ShaderStageFlagBits::eVertex );
		}

		inline static std::shared_ptr< Shader > loadFragment( std::filesystem::path path )
		{
			return loadShader( path, vk::ShaderStageFlagBits::eFragment );
		}

		//! Reloads the shader from disk
		void reload();
	};

} // namespace fgl::engine
