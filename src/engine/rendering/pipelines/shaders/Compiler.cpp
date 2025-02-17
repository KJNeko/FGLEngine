//
// Created by kj16609 on 8/5/24.
//

#include "Compiler.hpp"

#include <cassert>
#include <fstream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wuseless-cast"
#pragma GCC diagnostic ignored "-Wredundant-tags"
#pragma GCC diagnostic ignored "-Wcast-qual"
#pragma GCC diagnostic ignored "-Weffc++"
#include <slang-com-ptr.h>
#include <slang.h>
#pragma GCC diagnostic pop

#include "engine/FGL_DEFINES.hpp"
#include "engine/constants.hpp"
#include "engine/debug/logging/logging.hpp"
#include "rendering/pipelines/Shader.hpp"

namespace fgl::engine
{

	void checkDiag( Slang::ComPtr< slang::IBlob >& diag )
	{
		if ( !diag ) return;

		log::error( static_cast< const char* >( diag->getBufferPointer() ) );
		throw std::logic_error( "unexpected diagnostic" );
	}

	struct ShaderVertexFlags
	{};

	struct ShaderFragmentFlags
	{};

	using ShaderLayoutFlags = std::variant< ShaderVertexFlags, ShaderFragmentFlags >;

	void dumpJson( slang::ProgramLayout* layout )
	{
		Slang::ComPtr< slang::IBlob > json_glob {};
		layout->toJson( json_glob.writeRef() );

		log::debug(
			"Shader layout: {}",
			std::string_view(
				static_cast< const char* >( json_glob->getBufferPointer() ), json_glob->getBufferSize() ) );

		if ( std::ofstream ofs( "./shaders/slang-dump.json" ); ofs )
		{
			ofs.write( static_cast< const char* >( json_glob->getBufferPointer() ), json_glob->getBufferSize() );
		}
	}

	void checkMaterialDataLayout( slang::ProgramLayout* layout )
	{
		using namespace slang;

		auto* const type_info { layout->findTypeByName( "Material" ) };

		if ( type_info == nullptr ) return;

		if ( type_info->getKind() != TypeReflection::Kind::Struct )
		{
			throw std::logic_error( "unexpected type" );
		}

		//TODO: This
	}

	std::vector< std::byte > compileShader( const std::filesystem::path& path, const ShaderType type )
	{
		using namespace slang;

		static Slang::ComPtr< IGlobalSession > global_session {};
		static SlangGlobalSessionDesc global_desc {};

		const auto setupGlobalSession = [ & ]()
		{
			global_desc.enableGLSL = true;

			createGlobalSession( &global_desc, global_session.writeRef() );
		};

		static std::once_flag once {};
		std::call_once( once, setupGlobalSession );

		SessionDesc session_desc {};

		session_desc.defaultMatrixLayoutMode = SLANG_MATRIX_LAYOUT_COLUMN_MAJOR;

		std::array< CompilerOptionEntry, 1 > options {
			{ { CompilerOptionName::VulkanUseEntryPointName,
			    { .kind = CompilerOptionValueKind::Int, .intValue0 = 1 } } }
		};

		session_desc.compilerOptionEntries = options.data();
		session_desc.compilerOptionEntryCount = options.size();

		TargetDesc target_desc {};
		target_desc.format = SLANG_SPIRV;
		target_desc.profile = global_session->findProfile( "glsl_450" );

		FGL_ASSERT( target_desc.profile != SLANG_PROFILE_UNKNOWN, "Invalid profile" );

		session_desc.targets = &target_desc;
		session_desc.targetCount = 1;

		const auto search_path { std::filesystem::path() / "shaders" };
		const std::string search_path_str { search_path.string() };
		const char* const search_pathr_str_ptr { search_path_str.data() };
		session_desc.searchPaths = &search_pathr_str_ptr;
		session_desc.searchPathCount = 1;

		Slang::ComPtr< ISession > session {};
		global_session->createSession( session_desc, session.writeRef() );

		const auto module_name { path.filename().string() };

		Slang::ComPtr< IBlob > diagnostics;
		IModule* module { session->loadModule( module_name.c_str(), diagnostics.writeRef() ) };

		checkDiag( diagnostics );

		FGL_ASSERT( module != nullptr, "Invalid module" );

		std::string entry_point_name {};

		switch ( type )
		{
			case ShaderType::Compute:
				entry_point_name = "computeMain";
				break;
			case Vertex:
				entry_point_name = "vertexMain";
				break;
			case Fragment:
				entry_point_name = "fragmentMain";
				break;
			default:
				throw std::logic_error( "Invalid shader type" );
		}

		Slang::ComPtr< IEntryPoint > entry_point {};
		module->findEntryPointByName( entry_point_name.c_str(), entry_point.writeRef() );

		std::array< IComponentType*, 2 > components { { module, entry_point } };
		Slang::ComPtr< IComponentType > program;
		session->createCompositeComponentType(
			components.data(), components.size(), program.writeRef(), diagnostics.writeRef() );
		checkDiag( diagnostics );

		Slang::ComPtr< IComponentType > linked_program {};

		program->link( linked_program.writeRef(), diagnostics.writeRef() );
		checkDiag( diagnostics );

		const auto parent_path { path.parent_path() };

		{
			slang::ProgramLayout* layout { linked_program->getLayout() };

			Slang::ComPtr< IBlob > json_glob {};
			layout->toJson( json_glob.writeRef() );

#ifndef NDEBUG

			// dump the compilation layout json to a file
			std::filesystem::create_directory( parent_path / "dumps" );
			const std::string file_name { std::format( "{}-{}.json", path.stem().string(), entry_point_name ) };
			const auto target_path { parent_path / "dumps" / file_name };

			if ( std::ofstream ofs( target_path ); ofs )
			{
				ofs.write( static_cast< const char* >( json_glob->getBufferPointer() ), json_glob->getBufferSize() );
			}
#endif

			FGL_ASSERT( layout != nullptr, "Layout must be valid" );
		}

		int entry_index { 0 };
		int target_index { 0 };

		Slang::ComPtr< IBlob > kernel_blob {};
		linked_program->getEntryPointCode( entry_index, target_index, kernel_blob.writeRef(), diagnostics.writeRef() );
		checkDiag( diagnostics );

		FGL_ASSERT( kernel_blob != nullptr, "Kernel blob is not valid" );

		log::debug( "Compiled shader {} with a length of {}", path.filename().string(), kernel_blob->getBufferSize() );

		// if ( std::ofstream ofs( parent_path / std::format( "{}-{}.bin", path.filename().string(), entry_point_name ) );
		//      ofs )
		// {
		// 	ofs.write(
		// 		static_cast< const std::ostream::char_type* >( kernel_blob->getBufferPointer() ),
		// 		kernel_blob->getBufferSize() );
		// }

		std::vector< std::byte > compiled_code {};
		compiled_code.resize( kernel_blob->getBufferSize() );
		std::memcpy( compiled_code.data(), kernel_blob->getBufferPointer(), kernel_blob->getBufferSize() );

		return compiled_code;
	}

} // namespace fgl::engine
