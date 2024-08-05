//
// Created by kj16609 on 8/5/24.
//

#include "Compiler.hpp"

#include <cassert>

#include "engine/FGL_DEFINES.hpp"
#include "engine/logging/logging.hpp"

namespace fgl::engine
{

	inline static std::unique_ptr< shaderc::Compiler > compiler;

	shaderc::Compiler& getInstance()
	{
		if ( !compiler )
		{
			compiler = std::make_unique< shaderc::Compiler >();
		}

		FGL_ASSERT( compiler );

		return *compiler;
	}

	shaderc_shader_kind getShaderKindFromName( const std::string_view str )
	{
		if ( str.ends_with( ".frag" ) ) return shaderc_shader_kind::shaderc_fragment_shader;
		if ( str.ends_with( ".vert" ) ) return shaderc_shader_kind::shaderc_vertex_shader;

		throw std::runtime_error( "Unknown shader type!" );
	}

	std::vector< std::byte > compilerShader( const std::string_view input_name, std::vector< std::byte >& input )
	{
		shaderc::CompileOptions options;
#ifndef NDEBUG
		options.SetOptimizationLevel( shaderc_optimization_level_zero );
#else
		options.SetOptimizationLevel( shaderc_optimization_level_performance );
#endif

		const shaderc_shader_kind kind { getShaderKindFromName( input_name ) };

		const auto result { compiler->CompileGlslToSpvAssembly(
			reinterpret_cast< const char* >( input.data() ), input.size(), kind, input_name.data(), "main", options ) };

		switch ( result.GetCompilationStatus() )
		{
			case shaderc_compilation_status_success:
				break;
			default:
				[[fallthrough]];
			case shaderc_compilation_status_invalid_stage:
				[[fallthrough]];
			case shaderc_compilation_status_compilation_error:
				[[fallthrough]];
			case shaderc_compilation_status_internal_error:
				[[fallthrough]];
			case shaderc_compilation_status_null_result_object:
				[[fallthrough]];
			case shaderc_compilation_status_invalid_assembly:
				[[fallthrough]];
			case shaderc_compilation_status_validation_error:
				[[fallthrough]];
			case shaderc_compilation_status_transformation_error:
				[[fallthrough]];
			case shaderc_compilation_status_configuration_error:
				log::critical( "Failed to compile shader {} with error: {}", result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
				break;
		}

		std::vector< std::byte > output {};
		output.reserve( result.cend() - result.cbegin() );

		for ( const auto& word : result )
		{
			output.insert(
				output.end(),
				reinterpret_cast< const std::byte* >( &word ),
				reinterpret_cast< const std::byte* >( &word ) + sizeof( std::remove_reference_t< decltype( word ) > ) );
		}

		// Should be a multiple of 4
		assert( output.size() % 4 == 0 );

		return output;
	}

} // namespace fgl::engine
