//
// Created by kj16609 on 6/21/24.
//

#include "Shader.hpp"

#include <fstream>
#include <utility>

#include "engine/debug/logging/logging.hpp"
#include "engine/rendering/devices/Device.hpp"
#include "shaders/Compiler.hpp"

namespace fgl::engine
{

	std::string entrypointName( const ShaderType type, vk::PipelineShaderStageCreateInfo& info )
	{
		std::string str {};

		switch ( type )
		{
			case Vertex:
				str = "vertexMain";
				break;
			case Fragment:
				str = "fragmentMain";
				break;
			case Compute:
				str = "computeMain";
				break;
			default:
				throw std::runtime_error( "Invalid shader stage flags" );
		}

		info.setPName( str.c_str() );

		return str;
	}

	std::vector< std::byte > Shader::loadData( const std::filesystem::path& path, const ShaderType type )
	{
		if ( auto ifs = std::ifstream( path, std::ios::binary | std::ios::ate ); ifs )
		{
			return compileShader( path, type );
		}
		else
		{
			log::critical(
				"Failed to load shader module {}. Path not found", std::filesystem::absolute( path ).string() );
			throw std::runtime_error( "Failed to load shader module. Path not found" );
		}
	}

	vk::ShaderModuleCreateInfo Shader::createModuleInfo() const
	{
		vk::ShaderModuleCreateInfo module_info {};
		module_info.flags = {};
		module_info.codeSize = shader_data.size();
		module_info.pCode = reinterpret_cast< const std::uint32_t* >( shader_data.data() );

		return module_info;
	}

	Shader::Shader( std::filesystem::path path, const vk::PipelineShaderStageCreateInfo& info, const ShaderType type ) :
	  m_type( type ),
	  stage_info( info ),
	  m_entrypoint_name( entrypointName( type, stage_info ) ),
	  m_path( std::move( path ) ),
	  shader_data( loadData( m_path, type ) ),
	  module_create_info( createModuleInfo() ),
	  shader_module( Device::getInstance()->createShaderModule( module_create_info ) )
	{
		FGL_ASSERT( stage_info.pName == m_entrypoint_name.c_str(), "Entry point name mismatch" );
		log::debug( "Created shader {}", stage_info.pName );
		stage_info.module = shader_module;
	}

	std::shared_ptr< Shader > Shader::loadShader(
		const std::filesystem::path& path, const vk::ShaderStageFlagBits stage_flags, const ShaderType type )
	{
		std::filesystem::path full_path { std::filesystem::current_path() / path };

		vk::PipelineShaderStageCreateInfo stage_info {};
		stage_info.stage = stage_flags;
		stage_info.flags = {};
		stage_info.pName = nullptr;
		stage_info.pSpecializationInfo = VK_NULL_HANDLE;

		auto shader { std::make_shared< Shader >( path, stage_info, type ) };

		return shader;
	}

	void Shader::reload()
	{
		log::debug( "Reloading shader at {}", m_path.string() );
		shader_data = loadData( m_path, m_type );
		module_create_info = createModuleInfo();
		shader_module = Device::getInstance()->createShaderModule( module_create_info );
		stage_info.module = shader_module;
	}

} // namespace fgl::engine
