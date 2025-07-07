//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <filesystem>

#include "shaders/Compiler.hpp"

namespace fgl::engine
{

	struct Shader
	{
		ShaderType m_type;
		vk::PipelineShaderStageCreateInfo stage_info;
		std::string m_entrypoint_name;
		std::filesystem::path m_path;
		std::vector< std::byte > shader_data;
		vk::ShaderModuleCreateInfo module_create_info;

		vk::raii::ShaderModule shader_module;

		static std::vector< std::byte > loadData( const std::filesystem::path&, ShaderType type );
		vk::ShaderModuleCreateInfo createModuleInfo() const;

		Shader( std::filesystem::path path, const vk::PipelineShaderStageCreateInfo& info, ShaderType type );

		Shader( const Shader& other ) = delete;

		Shader& operator=( const Shader& other ) = delete;

		Shader( Shader&& other ) = delete;

		Shader& operator=( Shader&& other ) = delete;

		static std::shared_ptr< Shader >
			loadShader( const std::filesystem::path& path, vk::ShaderStageFlagBits stage_flags, ShaderType type );

		static std::shared_ptr< Shader > loadVertex( const std::filesystem::path& path )
		{
			return loadShader( path, vk::ShaderStageFlagBits::eVertex, ShaderType::Vertex );
		}

		static std::shared_ptr< Shader > loadFragment( const std::filesystem::path& path )
		{
			return loadShader( path, vk::ShaderStageFlagBits::eFragment, ShaderType::Fragment );
		}

		static std::shared_ptr< Shader > loadCompute( const std::filesystem::path& path )
		{
			return loadShader( path, vk::ShaderStageFlagBits::eCompute, ShaderType::Compute );
		}

		//! Reloads the shader from disk
		void reload();
	};

} // namespace fgl::engine
