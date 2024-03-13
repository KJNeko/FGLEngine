//
// Created by kj16609 on 3/13/24.
//

#ifndef SHADER_HPP
#define SHADER_HPP

#include <fstream>

#include "engine/descriptors/concepts.hpp"

namespace fgl::engine
{

	template < std::size_t N >
		requires( N > 0 )
	struct TString
	{
		using Character = std::string::value_type;

		Character str[ N ];

		constexpr TString( const char ( &literal )[ N ] ) { std::ranges::copy( literal, str ); }

		consteval operator std::string_view() const noexcept { return std::string_view( str, N - 1 ); }

		operator std::filesystem::path() const noexcept
		{
			return std::filesystem::path( std::string_view( str, N - 1 ) );
		}
	};

	struct ShaderHandle
	{
		vk::PipelineShaderStageCreateInfo stage_info;
		vk::ShaderModule shader_module;

		ShaderHandle( const std::filesystem::path path, const vk::PipelineShaderStageCreateInfo info ) :
		  stage_info( info ),
		  shader_module( VK_NULL_HANDLE )
		{
			if ( auto ifs = std::ifstream( path, std::ios::binary | std::ios::ate ); ifs )
			{
				std::vector< std::byte > data;
				data.resize( ifs.tellg() );
				ifs.seekg( 0, std::ios::beg );

				static_assert( sizeof( std::ifstream::char_type ) == sizeof( std::byte ) );
				ifs.read( reinterpret_cast< std::ifstream::char_type* >( data.data() ), data.size() );

				vk::ShaderModuleCreateInfo module_info {};
				module_info.flags = {};
				module_info.codeSize = data.size();
				module_info.pCode = reinterpret_cast< const std::uint32_t* >( data.data() );

				if ( Device::getInstance().device().createShaderModule( &module_info, nullptr, &shader_module )
				     != vk::Result::eSuccess )
					throw std::runtime_error( "Failed to create shader module" );

				std::cout << "Created shader module for: " << path << std::endl;

				stage_info.module = shader_module;
			}
			else
				throw std::runtime_error( "Failed to load shader module. Path not found" );
		}

		ShaderHandle( const ShaderHandle& other ) = delete;

		ShaderHandle& operator=( const ShaderHandle& other ) = delete;

		ShaderHandle( ShaderHandle&& other ) = delete;

		ShaderHandle& operator=( ShaderHandle&& other ) = delete;

		~ShaderHandle()
		{
			if ( shader_module != VK_NULL_HANDLE ) Device::getInstance().device().destroyShaderModule( shader_module );
		}
	};

	vk::ShaderModule loadShaderModule( const std::string_view path );

	template < TString filepath, vk::ShaderStageFlagBits stage_flags >
	struct Shader
	{
		consteval static vk::PipelineShaderStageCreateInfo defaultShaderInfo()
		{
			vk::PipelineShaderStageCreateInfo info {};
			info.flags = {};
			info.stage = stage_flags;
			info.pName = "main";
			info.pSpecializationInfo = nullptr;

			return info;
		}

		static std::unique_ptr< ShaderHandle > load()
		{
			return std::make_unique< ShaderHandle >( filepath, defaultShaderInfo() );
		}

		virtual ~Shader() = default;
	};

	template < TString filepath >
	using VertexShaderT = Shader< filepath, vk::ShaderStageFlagBits::eVertex >;

	template < TString filepath >
	using FragmentShaderT = Shader< filepath, vk::ShaderStageFlagBits::eFragment >;

	template < typename T >
	concept is_shader = requires( T t ) {
		{
			t.defaultShaderInfo()
		} -> std::same_as< vk::PipelineShaderStageCreateInfo >;
	};

	template < is_shader... Shaders >
	struct ShaderCollection
	{
		using ShaderTuple = std::tuple< Shaders... >;

		constexpr static std::uint64_t ShaderCount { sizeof...( Shaders ) };

		template < std::uint64_t IDX >
			requires( IDX < ShaderCount )
		using Shader = std::tuple_element_t< IDX, ShaderTuple >;

		static_assert( ShaderCount >= 2, "Shader count must be two, Missing vertex or fragment?" );

		static std::vector< std::unique_ptr< ShaderHandle > > loadShaders()
		{
			std::vector< std::unique_ptr< ShaderHandle > > shaders;

			( ( shaders.push_back( Shaders::load() ) ), ... );

			return shaders;
		}
	};

	template < typename T >
	concept is_shader_collection = requires( T t ) {
		typename T::ShaderTuple;
		{
			t.ShaderCount
		} -> std::same_as< const std::uint64_t& >;
	};

} // namespace fgl::engine

#endif //SHADER_HPP
