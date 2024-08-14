//
// Created by kj16609 on 6/21/24.
//

#include "Shader.hpp"

#include "engine/rendering/devices/Device.hpp"
#include "shaders/Compiler.hpp"

namespace fgl::engine
{

	std::vector< std::byte > ShaderHandle::loadData( const std::filesystem::path& path )
	{
		if ( auto ifs = std::ifstream( path, std::ios::binary | std::ios::ate ); ifs )
		{
			std::vector< std::byte > data;
			data.resize( ifs.tellg() );
			ifs.seekg( 0, std::ios::beg );

			static_assert( sizeof( std::ifstream::char_type ) == sizeof( std::byte ) );

			ifs.read( reinterpret_cast< std::ifstream::char_type* >( data.data() ), data.size() );

			// We now need to compile the shader before we use it.

			return compileShader( path.filename().string(), data );
		}
		else
		{
			log::critical( "Failed to load shader module {}. Path not found", path.string() );
			throw std::runtime_error( "Failed to load shader module. Path not found" );
		}
	}

	vk::ShaderModuleCreateInfo ShaderHandle::createModuleInfo()
	{
		vk::ShaderModuleCreateInfo module_info {};
		module_info.flags = {};
		module_info.codeSize = shader_data.size();
		module_info.pCode = reinterpret_cast< const std::uint32_t* >( shader_data.data() );

		return module_info;
	}

	ShaderHandle::ShaderHandle( const std::filesystem::path& path, const vk::PipelineShaderStageCreateInfo& info ) :
	  shader_data( loadData( path ) ),
	  module_create_info( createModuleInfo() ),
	  stage_info( info ),
	  shader_module( Device::getInstance()->createShaderModule( module_create_info ) )
	{
		stage_info.module = shader_module;
	}

} // namespace fgl::engine
