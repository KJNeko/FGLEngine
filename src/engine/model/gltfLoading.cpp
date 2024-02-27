//
// Created by kj16609 on 2/5/24.
//

#include "Model.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "engine/image/Sampler.hpp"
#include "objectloaders/tiny_gltf.h"
#pragma GCC diagnostic pop

#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/image/ImageView.hpp"

namespace fgl::engine
{

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
			const auto& buffer_view { model.bufferViews.at( accessor.bufferView ) };
			const auto& buffer { model.buffers.at( buffer_view.buffer ) };

			std::vector< T > data {};
			data.reserve( accessor.count );

			std::uint16_t copy_size { 0 };
			switch ( accessor.componentType )
			{
				default:
					throw std::runtime_error( "Unhandled access size" );
				case TINYGLTF_COMPONENT_TYPE_FLOAT:
					copy_size = 32 / 8;
					break;
				case TINYGLTF_COMPONENT_TYPE_BYTE:
					copy_size = 8 / 8;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					copy_size = 32 / 8;
					break;
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					copy_size = 16 / 8;
					break;
			}

			switch ( accessor.type )
			{
				default:
					throw std::runtime_error( "Unhandled access type" );
				case TINYGLTF_TYPE_VEC3:
					copy_size *= 3;
					break;
				case TINYGLTF_TYPE_VEC2:
					copy_size *= 2;
					break;
				case TINYGLTF_TYPE_SCALAR:
					copy_size *= 1;
					break;
			}

			constexpr auto T_SIZE { sizeof( T ) };

			if ( T_SIZE != copy_size )
				throw std::runtime_error(
					std::string( "Accessor copy values not matching sizeof(T): sizeof(" )
					+ std::string( typeid( T ).name() ) + ") == " + std::to_string( T_SIZE )
					+ " vs copy_size = " + std::to_string( copy_size ) );

			const auto real_size { copy_size * accessor.count };

			data.resize( accessor.count );

			std::memcpy( data.data(), buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset, real_size );

			return data;
		}
	}

	void ModelBuilder::loadGltf( const std::filesystem::path& filepath )
	{
		std::cout << "Loading gltf model " << filepath << std::endl;

		if ( !std::filesystem::exists( filepath ) ) throw std::runtime_error( "File does not exist" );

		m_primitives.clear();

		tinygltf::Model model {};
		tinygltf::TinyGLTF loader {};
		std::string err {};
		std::string warn {};

		loader.RemoveImageLoader();

		loader.LoadASCIIFromFile( &model, &err, &warn, filepath.string() );

		if ( !err.empty() ) throw std::runtime_error( err );

		if ( !warn.empty() )
			std::cout << "Warning while loading model \"" << filepath.string() << "\"\nWarning:" << warn << std::endl;

		for ( const tinygltf::Mesh& mesh : model.meshes )
		{
			std::vector< glm::vec3 > model_positions;

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
				std::vector< ModelCoordinate > position_data {
					extractData< ModelCoordinate >( model, position_accessor )
				};
				model_positions.insert( model_positions.end(), position_data.begin(), position_data.end() );

				ModelBoundingBox bounding_box { generateBoundingFromPoints( position_data ) };

				std::vector< glm::vec3 > normals {};

				if ( primitive.attributes.find( "NORMAL" ) != primitive.attributes.end() )
				{
					normals =
						extractData< glm::vec3 >( model, model.accessors.at( primitive.attributes.at( "NORMAL" ) ) );
				}
				else // TODO: Precompute normals if required
					normals.resize( position_data.size() );

				//TODO: Implement TANGENT reading
				std::vector< glm::vec3 > tangents {};

				std::vector< glm::vec2 > texcoords {};

				for ( const auto& [ attr_name, attr_idx ] : primitive.attributes )
				{
					if ( attr_name.starts_with( "TEXCOORD_" ) )
					{
						//Rip out name and figure out index
						const auto idx { std::stoi( attr_name.substr( strlen( "TEXCOORD_" ) ) ) };

						if ( idx != 0 ) throw std::runtime_error( "Multiple tex coordinates not supported" );

						const auto& texcoord_accessor { model.accessors.at( attr_idx ) };

						texcoords = extractData< glm::vec2 >( model, texcoord_accessor );
					}
				}

				std::vector< Vertex > verts {};
				verts.resize( position_data.size() );

				assert( verts.size() == normals.size() );

				for ( std::size_t i = 0; i < position_data.size(); i++ )
				{
					//Fix position to be -Z UP
					//verts[ i ].m_position = position_data[ i ];
					verts[ i ].m_position =
						glm::vec3( position_data[ i ].x, position_data[ i ].z, position_data[ i ].y );
					verts[ i ].m_normal = normals[ i ];
				}

				if ( texcoords.size() > 0 && texcoords.size() != verts.size() )
				{
					throw std::runtime_error(
						"wtf? " + std::to_string( texcoords.size() ) + " < " + std::to_string( verts.size() ) );
				}

				if ( texcoords.size() == verts.size() )
				{
					for ( std::size_t i = 0; i < texcoords.size(); ++i ) verts[ i ].m_uv = texcoords[ i ];
				}

				VertexBufferSuballocation vertex_buffer { m_vertex_buffer, verts };
				IndexBufferSuballocation index_buffer { m_index_buffer, indicies_data };

				if ( primitive.material >= 0 && primitive.material < static_cast< int >( model.materials.size() ) )
				{
					const auto& material { model.materials.at( static_cast< unsigned long >( primitive.material ) ) };

					//TODO: Implement material normals

					//Color texture
					if ( material.values.contains( "baseColorTexture" ) )
					{
						const auto& color_texture { material.values.at( "baseColorTexture" ) };
						const auto color_index { color_texture.TextureIndex() };

						const auto& texture { model.textures.at( static_cast< unsigned long >( color_index ) ) };

						const auto& source { model.images.at( static_cast< unsigned long >( texture.source ) ) };
						const auto& sampler { model.samplers.at( static_cast< unsigned long >( texture.sampler ) ) };

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

						Primitive prim {
							std::move( vertex_buffer ), std::move( index_buffer ), bounding_box, std::move( tex )
						};

						m_primitives.emplace_back( std::move( prim ) );

						continue;
					}
				}
				else
					std::cout << "No material" << std::endl;

				Primitive prim { std::move( vertex_buffer ), std::move( index_buffer ), bounding_box };

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
} // namespace fgl::engine
