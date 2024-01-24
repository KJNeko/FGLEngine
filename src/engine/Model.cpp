//
// Created by kj16609 on 11/28/23.
//

#include "Model.hpp"

#include <cassert>
#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtx/hash.hpp>

#include <iostream>
#include <unordered_map>

#include "engine/buffers/Buffer.hpp"
#include "engine/buffers/SuballocationView.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/ImageView.hpp"
#include "engine/image/Sampler.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "objectloaders/tiny_gltf.h"
#include "objectloaders/tiny_obj_loader.h"
#pragma GCC diagnostic pop

#include "utils.hpp"

namespace std
{
	template <>
	struct hash< fgl::engine::Vertex >
	{
		size_t operator()( const fgl::engine::Vertex& vertex ) const
		{
			std::size_t seed { 0 };
			fgl::engine::hashCombine( seed, vertex.m_position, vertex.m_color, vertex.m_normal, vertex.m_uv );
			return seed;
		}
	};

} // namespace std

namespace fgl::engine
{

	std::vector< vk::DrawIndexedIndirectCommand > Model::buildParameters( const std::vector< Primitive >& primitives )
	{
		std::vector< vk::DrawIndexedIndirectCommand > draw_parameters;

		for ( const auto& primitive : primitives )
		{
			vk::DrawIndexedIndirectCommand cmd;
			cmd.indexCount = primitive.m_index_buffer.count();
			cmd.firstIndex = primitive.m_index_buffer.getOffsetCount();

			cmd.vertexOffset = static_cast< std::int32_t >( primitive.m_vertex_buffer.getOffsetCount() );

			cmd.firstInstance = 0;
			cmd.instanceCount = 1;

			draw_parameters.emplace_back( std::move( cmd ) );
		}

		return draw_parameters;
	}

	std::vector< vk::DrawIndexedIndirectCommand > Model::getDrawCommand( const std::uint32_t index ) const
	{
		std::vector< vk::DrawIndexedIndirectCommand > draw_commands;
		draw_commands.reserve( m_primitives.size() );
		for ( const auto& cmd : m_draw_parameters )
		{
			auto new_cmd { cmd };
			new_cmd.firstInstance = index;

			draw_commands.push_back( new_cmd );
		}

		return draw_commands;
	}

	Model::Model( Device& device, Builder& builder ) :
	  m_device( device ),
	  m_draw_parameters( buildParameters( builder.m_primitives ) )
	{
		m_primitives = std::move( builder.m_primitives );
	}

	std::unique_ptr< Model > Model::
		createModel( Device& device, const std::filesystem::path& path, Buffer& vertex_buffer, Buffer& index_buffer )
	{
		Builder builder { vertex_buffer, index_buffer };
		builder.loadModel( path );

		return std::make_unique< Model >( device, builder );
	}

	void Model::syncBuffers( vk::CommandBuffer& cmd_buffer )
	{
		for ( auto& primitive : m_primitives )
		{
			primitive.m_vertex_buffer.stage( cmd_buffer );
			primitive.m_index_buffer.stage( cmd_buffer );
		}
	}

	std::vector< vk::VertexInputBindingDescription > Vertex::getBindingDescriptions()
	{
		std::vector< vk::VertexInputBindingDescription > binding_descriptions {
			{ 0, sizeof( Vertex ), vk::VertexInputRate::eVertex },
			{ 1, sizeof( ModelMatrixInfo ), vk::VertexInputRate::eInstance }
		};

		return binding_descriptions;
	}

	std::vector< vk::VertexInputAttributeDescription > Vertex::getAttributeDescriptions()
	{
		std::vector< vk::VertexInputAttributeDescription > attribute_descriptions {};

		attribute_descriptions.emplace_back( 0, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_position ) );
		attribute_descriptions.emplace_back( 1, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_color ) );
		attribute_descriptions.emplace_back( 2, 0, vk::Format::eR32G32B32Sfloat, offsetof( Vertex, m_normal ) );
		attribute_descriptions.emplace_back( 3, 0, vk::Format::eR32G32Sfloat, offsetof( Vertex, m_uv ) );

		//Normal Matrix
		attribute_descriptions.emplace_back( 4, 1, vk::Format::eR32G32B32A32Sfloat, 0 );
		attribute_descriptions.emplace_back( 5, 1, vk::Format::eR32G32B32A32Sfloat, 1 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 6, 1, vk::Format::eR32G32B32A32Sfloat, 2 * sizeof( glm::vec4 ) );
		attribute_descriptions.emplace_back( 7, 1, vk::Format::eR32G32B32A32Sfloat, 3 * sizeof( glm::vec4 ) );

		attribute_descriptions.emplace_back( 8, 1, vk::Format::eR32Sint, 4 * sizeof( glm::vec4 ) );

		static_assert( 4 * sizeof( glm::vec4 ) + sizeof( int ) == sizeof( ModelMatrixInfo ) );

		return attribute_descriptions;
	}

	void Model::Builder::loadModel( const std::filesystem::path& filepath )
	{
		if ( filepath.extension() == ".obj" )
		{
			loadObj( filepath );
		}
		else if ( filepath.extension() == ".gltf" )
		{
			loadGltf( filepath );
		}
		else
			//Dunno
			throw std::runtime_error( "Unknown model file extension" );
	}

	template < typename T >
	std::vector< T > extractData( const tinygltf::Model& model, const tinygltf::Accessor& accessor )
	{
		if ( accessor.sparse.isSparse )
		{
			//Sparse loading required

			throw std::runtime_error( "Sparse loading not implemeneted" );
		}
		else
		{
			auto& buffer_view { model.bufferViews.at( accessor.bufferView ) };
			auto& buffer { model.buffers.at( buffer_view.buffer ) };

			std::vector< T > data;

			std::uint16_t copy_size { 0 };
			switch ( accessor.componentType )
			{
				default:
					throw std::runtime_error( "Unhandled access size" );
				case TINYGLTF_COMPONENT_TYPE_FLOAT:
					copy_size = sizeof( float );
					break;
				case TINYGLTF_COMPONENT_TYPE_BYTE:
					copy_size = sizeof( std::byte );
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					copy_size = sizeof( unsigned int );
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					copy_size = sizeof( unsigned short );
					break;
			}

			switch ( accessor.type )
			{
				default:
					throw std::runtime_error( "UNhandled access type" );
				case TINYGLTF_TYPE_VEC3:
					copy_size *= 3;
					break;
				case TINYGLTF_TYPE_VEC2:
					copy_size *= 2;
					break;
				case TINYGLTF_TYPE_SCALAR:
					//noop
					break;
			}

			constexpr auto T_SIZE { sizeof( T ) };

			assert( T_SIZE == copy_size && "Accessor copy values not greater than or matching sizeof(T)" );

			const auto real_size { copy_size * accessor.count };

			data.resize( real_size );

			std::memcpy( data.data(), buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset, real_size );

			return data;
		}
	};

	void Model::Builder::loadGltf( const std::filesystem::path& filepath )
	{
		std::cout << "Loading gltf model " << filepath << std::endl;

		if ( !std::filesystem::exists( filepath ) ) throw std::runtime_error( "File does not exist" );

		m_primitives.clear();

		tinygltf::Model model {};
		tinygltf::TinyGLTF loader {};
		std::string err;
		std::string warn;

		loader.RemoveImageLoader();

		loader.LoadASCIIFromFile( &model, &err, &warn, filepath.string() );

		if ( !err.empty() ) throw std::runtime_error( err );

		if ( !warn.empty() )
			std::cout << "Warning while loading model \"" << filepath.string() << "\"\nWarning:" << warn << std::endl;

		for ( const tinygltf::Mesh& mesh : model.meshes )
		{
			for ( const tinygltf::Primitive& primitive : mesh.primitives )
			{
				//TODO: Implement modes

				std::cout << "Attributes: \n";
				for ( const auto& thing : primitive.attributes )
				{
					std::cout << "\t" << thing.first << "\n";
				}
				std::cout << std::endl;

				//Load indicies
				auto& indicies_accessor { model.accessors.at( primitive.indices ) };

				std::vector< std::uint32_t > indicies_data {};

				if ( indicies_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT )
				{
					indicies_data = extractData< std::uint32_t >( model, model.accessors.at( primitive.indices ) );
				}
				else if ( indicies_accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT )
				{
					auto tmp { extractData< std::uint16_t >( model, model.accessors.at( primitive.indices ) ) };

					indicies_data.reserve( tmp.size() );

					for ( auto& val : tmp )
					{
						indicies_data.emplace_back( val );
					}
				}

				//Load positions
				auto& position_accessor { model.accessors.at( primitive.attributes.at( "POSITION" ) ) };
				std::vector< glm::vec3 > position_data { extractData< glm::vec3 >( model, position_accessor ) };

				std::vector< glm::vec3 > normals;

				if ( primitive.attributes.find( "NORMAL" ) != primitive.attributes.end() )
				{
					normals =
						extractData< glm::vec3 >( model, model.accessors.at( primitive.attributes.at( "NORMAL" ) ) );
				}
				else // TODO: Precompute normals if required
					normals.resize( position_data.size() );

				//TODO: Implement TANGENT reading
				std::vector< glm::vec3 > tangents;

				std::vector< glm::vec2 > texcoords;

				for ( const auto& [ attr_name, attr_idx ] : primitive.attributes )
				{
					if ( attr_name.starts_with( "TEXCOORD_" ) )
					{
						//Rip out name and figure out index
						const auto idx { std::stoi( attr_name.substr( strlen( "TEXCOORD_" ) ) ) };

						if ( idx != 0 ) throw std::runtime_error( "Multiple tex coordinates not supported" );

						auto& texcoord_accessor { model.accessors.at( attr_idx ) };

						texcoords = extractData< glm::vec2 >( model, texcoord_accessor );
					}
				}

				std::vector< Vertex > verts;
				verts.resize( position_data.size() );
				for ( std::size_t i = 0; i < position_data.size(); i++ )
				{
					//Fix position to be -Z UP
					//verts[ i ].m_position = position_data[ i ];
					verts[ i ].m_position = { position_data[ i ].x, -position_data[ i ].y, position_data[ i ].z };
					verts[ i ].m_normal = normals[ i ];
					verts[ i ].m_uv = texcoords[ i ];
				}

				VertexBufferSuballocation vertex_buffer { m_vertex_buffer, verts };
				IndexBufferSuballocation index_buffer { m_index_buffer, indicies_data };

				if ( primitive.material >= 0 && primitive.material < model.materials.size() )
				{
					const auto& material { model.materials.at( primitive.material ) };

					//TODO: Implement material normals

					//Color texture
					if ( material.values.contains( "baseColorTexture" ) )
					{
						const auto& color_texture { material.values.at( "baseColorTexture" ) };
						const auto color_index { color_texture.TextureIndex() };

						const auto& texture { model.textures.at( color_index ) };

						const auto& source { model.images.at( texture.source ) };
						const auto& sampler { model.samplers.at( texture.sampler ) };

						auto translateFilterToVK = []( const int val ) -> vk::Filter
						{
							switch ( val )
							{
								default:
									throw std::runtime_error( "Failed to translate filter value to vk filter value" );
								case GL_NEAREST:
									return vk::Filter::eNearest;
								case GL_LINEAR:
									[[fallthrough]];
								case GL_LINEAR_MIPMAP_LINEAR:
									return vk::Filter::eLinear;
							}
						};

						auto translateWarppingToVk = []( const int val ) -> vk::SamplerAddressMode
						{
							switch ( val )
							{
								default:
									throw std::
										runtime_error( "Failed to translate wrapping filter to vk address mode" );
								case GL_REPEAT:
									return vk::SamplerAddressMode::eRepeat;
#ifdef GL_CLAMP_TO_BORDER
								case GL_CLAMP_TO_BORDER:
									return vk::SamplerAddressMode::eClampToBorder;
#endif
#ifdef GL_CLAMP_TO_EDGE
								case GL_CLAMP_TO_EDGE:
									return vk::SamplerAddressMode::eClampToEdge;
#endif
							}
						};

						assert(
							sampler.wrapS == sampler.wrapT
							&& "Can't support different wrap modes for textures on each axis" );

						Texture tex { Texture::loadFromFile( filepath.parent_path() / source.uri ) };
						Sampler smp { translateFilterToVK( sampler.minFilter ),
							          translateFilterToVK( sampler.magFilter ),
							          vk::SamplerMipmapMode::eLinear,
							          translateWarppingToVk( sampler.wrapS ) };

						tex.getImageView().getSampler() = std::move( smp );
						tex.createImGuiSet();

						Texture::getTextureDescriptorSet().bindTexture( 0, tex );
						Texture::getTextureDescriptorSet().update();

						//Stage texture
						auto cmd { Device::getInstance().beginSingleTimeCommands() };

						tex.stage( cmd );
						Device::getInstance().endSingleTimeCommands( cmd );
						tex.dropStaging();

						Primitive prim { std::move( vertex_buffer ), std::move( index_buffer ), std::move( tex ) };

						m_primitives.emplace_back( std::move( prim ) );

						continue;
					}
				}
				else
					std::cout << "No material" << std::endl;

				Primitive prim { std::move( vertex_buffer ), std::move( index_buffer ) };

				m_primitives.emplace_back( std::move( prim ) );
			}

			std::cout << "Mesh has " << mesh.primitives.size() << " primitives" << std::endl;
		}

		for ( const tinygltf::Scene& scene : model.scenes )
		{
			std::cout << "Loading scene " << scene.name << std::endl;
			std::cout << "Scene has " << scene.nodes.size() << " nodes" << std::endl;

			for ( auto child : scene.nodes )
			{
				std::cout << "Child: " << child << std::endl;
			}
		}

		std::cout << "Scenes: " << model.scenes.size() << std::endl;

		std::cout << "Meshes: " << model.meshes.size() << std::endl;
	}

	void Model::Builder::loadObj( const std::filesystem::path& filepath )
	{
		m_primitives.clear();

		tinyobj::attrib_t attrib {};
		std::vector< tinyobj::shape_t > shapes {};
		std::vector< tinyobj::material_t > materials {};
		std::string warn {};
		std::string error {};

		if ( !tinyobj::LoadObj( &attrib, &shapes, &materials, &warn, &error, filepath.string< char >().c_str() ) )
			throw std::runtime_error( warn + error );

		std::unordered_map< Vertex, std::uint32_t > unique_verts {};

		std::vector< Vertex > verts;
		std::vector< std::uint32_t > indicies;

		for ( const auto& shape : shapes )
		{
			for ( const auto& index : shape.mesh.indices )
			{
				Vertex vert {};
				if ( index.vertex_index >= 0 )
				{
					vert.m_position = {
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 0 ) ],
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 1 ) ],
						attrib.vertices[ static_cast< std::uint64_t >( 3 * index.vertex_index + 2 ) ],
					};

					vert.m_color = { attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 0 ) ],
						             attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 1 ) ],
						             attrib.colors[ static_cast< std::uint64_t >( 3 * index.vertex_index + 2 ) ] };

					assert( vert.m_color[ 0 ] > 0.2f );
					assert( vert.m_color[ 1 ] > 0.2f );
					assert( vert.m_color[ 2 ] > 0.2f );
				}

				if ( index.normal_index >= 0 )
				{
					vert.m_normal = {
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 0 ) ],
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 1 ) ],
						attrib.normals[ static_cast< std::uint64_t >( 3 * index.normal_index + 2 ) ],
					};
				}

				if ( index.texcoord_index >= 0 )
				{
					vert.m_uv = {
						attrib.texcoords[ static_cast< std::uint64_t >( 3 * index.texcoord_index + 0 ) ],
						attrib.texcoords[ static_cast< std::uint64_t >( 3 * index.texcoord_index + 1 ) ],
					};
				}

				if ( auto itter = unique_verts.find( vert ); itter != unique_verts.end() )
				{
					indicies.push_back( unique_verts[ vert ] );
				}
				else
				{
					const auto insert_op { unique_verts.insert( std::make_pair( vert, verts.size() ) ) };
					verts.emplace_back( vert );

					if ( insert_op.second )
						indicies.push_back( insert_op.first->second );
					else
						throw std::runtime_error( "Failed to insert new vertex" );
				}
			}
		}

		m_primitives.emplace_back(
			VertexBufferSuballocation( m_vertex_buffer, std::move( verts ) ),
			IndexBufferSuballocation( m_index_buffer, std::move( indicies ) ) );

		std::cout << unique_verts.size() << " unique verts" << std::endl;
	}
} // namespace fgl::engine