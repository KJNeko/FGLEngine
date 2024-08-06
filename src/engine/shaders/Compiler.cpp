//
// Created by kj16609 on 8/5/24.
//

#include "Compiler.hpp"

#include <cassert>
#include <fstream>

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

	class Includer : public shaderc::CompileOptions::IncluderInterface
	{
		struct DataHolder
		{
			std::filesystem::path source_path {};
			std::vector< char > content {};
		};

		shaderc_include_result* GetInclude(
			const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth ) override;

		void ReleaseInclude( shaderc_include_result* data ) override;
	};

	shaderc_include_result* Includer::GetInclude(
		const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth )
	{
		const std::string_view requsted { requested_source };
		const std::string_view requster { requesting_source };
		log::debug( "Source file {} is requesting {}", requster, requsted );

		std::vector< char > file_data {};

		const auto path { std::filesystem::current_path() / "shaders" / requsted };

		if ( std::ifstream ifs( path ); ifs )
		{
			file_data.resize( std::filesystem::file_size( path ) );

			ifs.read( file_data.data(), file_data.size() );
			log::debug( "Found source file {} to be included into {}", requsted, requster );
		}
		else
		{
			log::error( "Failed to find include {} for {}", path, requster );
			//throw std::runtime_error( std::format( "Failed to open include file {} for file {}", requsted, requster ) );

			auto error_return { new shaderc_include_result() };
			error_return->user_data = nullptr;
			error_return->source_name = "";
			error_return->source_name_length = 0;
			error_return->content = "Failed to find include for requsted file";

			return error_return;
		}

		auto* data_holder { new DataHolder( path, std::move( file_data ) ) };

		auto data { new shaderc_include_result() };
		data->user_data = data_holder;
		data->content = data_holder->content.data();
		data->content_length = data_holder->content.size();
		data->source_name = data_holder->source_path.c_str();
		data->source_name_length = data_holder->source_path.string().size();

		return data;
	}

	void Includer::ReleaseInclude( shaderc_include_result* data )
	{
		delete static_cast< DataHolder* >( data->user_data );
		delete data;
	}

	std::vector< std::byte > compileShader( const std::string_view input_name, const std::vector< std::byte >& input )
	{
		shaderc::CompileOptions options {};

		options.SetTargetEnvironment( shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3 );

		options.SetIncluder( std::make_unique< Includer >() );

#ifndef NDEBUG
		options.SetOptimizationLevel( shaderc_optimization_level_zero );
#else
		options.SetOptimizationLevel( shaderc_optimization_level_performance );
#endif

		options.SetVulkanRulesRelaxed( false );

		const shaderc_shader_kind kind { getShaderKindFromName( input_name ) };

		const auto preprocessed_source { getInstance().PreprocessGlsl(
			reinterpret_cast< const char* >( input.data() ), input.size(), kind, input_name.data(), options ) };

		log::info(
			"Preprocessed source:\n{}", std::string_view( preprocessed_source.begin(), preprocessed_source.end() ) );

		const auto result { getInstance().CompileGlslToSpv(
			reinterpret_cast< const char* >( input.data() ), input.size(), kind, input_name.data(), options ) };

		switch ( result.GetCompilationStatus() )
		{
			case shaderc_compilation_status_success:
				break;
			case shaderc_compilation_status_compilation_error:
				log::critical(
					"Compilation error when compiling shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
			default:
				[[fallthrough]];
			case shaderc_compilation_status_internal_error:
				log::critical(
					"internal error while compiling shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
				[[fallthrough]];
			case shaderc_compilation_status_null_result_object:
				log::critical(
					"null result object when compiling shader {} with error: {}",
					input_name,
					result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
				[[fallthrough]];
			case shaderc_compilation_status_invalid_assembly:
				log::critical( "Failed to compile shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
				[[fallthrough]];
			case shaderc_compilation_status_validation_error:
				log::critical( "Failed to compile shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
			case shaderc_compilation_status_transformation_error:
				log::critical( "Failed to compile shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
			case shaderc_compilation_status_invalid_stage:
				log::critical( "Failed to compile shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
			case shaderc_compilation_status_configuration_error:
				log::critical( "Failed to compile shader {} with error: {}", input_name, result.GetErrorMessage() );
				throw std::runtime_error( "Failed to compile shader" );
				break;
		}

		log::debug( "Compiled shader {}", input_name );

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
		assert( output.size() > 0 );

		return output;
	}

} // namespace fgl::engine
