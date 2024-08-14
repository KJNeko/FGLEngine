//
// Created by kj16609 on 3/13/24.
//

#pragma once

#include <vulkan/vulkan_raii.hpp>

#include <fstream>

#include "../../logging/logging.hpp"

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
		std::vector< std::byte > shader_data;
		vk::ShaderModuleCreateInfo module_create_info;
		vk::PipelineShaderStageCreateInfo stage_info;

		vk::raii::ShaderModule shader_module;

		std::vector< std::byte > loadData( const std::filesystem::path& );
		vk::ShaderModuleCreateInfo createModuleInfo();

		ShaderHandle( const std::filesystem::path& path, const vk::PipelineShaderStageCreateInfo& info );

		ShaderHandle( const ShaderHandle& other ) = delete;

		ShaderHandle& operator=( const ShaderHandle& other ) = delete;

		ShaderHandle( ShaderHandle&& other ) = delete;

		ShaderHandle& operator=( ShaderHandle&& other ) = delete;
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

	template < TString filepath >
	using TesselationControlShaderT = Shader< filepath, vk::ShaderStageFlagBits::eTessellationControl >;

	template < TString filepath >
	using TesselationEvaluationShaderT = Shader< filepath, vk::ShaderStageFlagBits::eTessellationEvaluation >;

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

		static_assert( ShaderCount >= 1, "Shader count must be two, Missing vertex or fragment?" );

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
