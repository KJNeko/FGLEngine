//
// Created by kj16609 on 5/18/24.
//

#include "SceneBuilder.hpp"

#include "engine/assets/model/Model.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include "objectloaders/tiny_gltf.h"
#pragma GCC diagnostic pop

#include "engine/assets/image/ImageView.hpp"
#include "engine/assets/stores.hpp"
#include "engine/camera/Camera.hpp"
#include "engine/debug/logging/logging.hpp"
#include "engine/descriptors/DescriptorSet.hpp"
#include "engine/gameobjects/GameObject.hpp"

namespace fgl::engine
{

	SceneBuilder::SceneBuilder( memory::Buffer& vertex_buffer, memory::Buffer& index_buffer ) :
	  m_vertex_buffer( vertex_buffer ),
	  m_index_buffer( index_buffer )
	{}

	int SceneBuilder::getTexcoordCount( const tinygltf::Primitive& prim ) const
	{
		ZoneScoped;
		int counter { 0 };
		for ( const auto& [ key, value ] : prim.attributes )
		{
			if ( key.starts_with( "TEXCOORD" ) ) ++counter;
		}
		return counter;
	}

	template < typename T >
	std::vector< T > extractData( const tinygltf::Model& model, const tinygltf::Accessor& accessor )
	{
		ZoneScoped;
		if ( accessor.sparse.isSparse )
		{
			//Sparse loading required
			throw std::runtime_error( "Sparse loading not implemeneted" );
		}

		const auto& buffer_view { model.bufferViews.at( accessor.bufferView ) };
		const auto& buffer { model.buffers.at( buffer_view.buffer ) };

		std::vector< T > data {};
		data.reserve( accessor.count );

		std::uint16_t byte_count { 0 };
		switch ( accessor.componentType )
		{
			default:
				throw std::runtime_error( "Unhandled access size" );
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				byte_count = 32 / 8;
				break;
			case TINYGLTF_COMPONENT_TYPE_BYTE:
				byte_count = 8 / 8;
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				byte_count = 32 / 8;
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				byte_count = 16 / 8;
				break;
		}

		switch ( accessor.type )
		{
			default:
				throw std::runtime_error( "Unhandled access type" );
			case TINYGLTF_TYPE_VEC3:
				byte_count *= 3;
				break;
			case TINYGLTF_TYPE_VEC2:
				byte_count *= 2;
				break;
			case TINYGLTF_TYPE_SCALAR:
				byte_count *= 1;
				break;
		}

		// Size of the type we are extracting into
		constexpr auto T_SIZE { sizeof( T ) };

		// If the type size is smaller then we need. Then we need to throw an error
		if ( T_SIZE != byte_count )
		{
			// If the type is scalar type we can still safely use it without any major worries
			if ( accessor.type == TINYGLTF_TYPE_SCALAR && T_SIZE >= byte_count )
			{
				// If the type is a smaller scalar then we want can still copy the data.
				// log::warn( "Attempting to copy data of size {} into type of size {}", byte_count, T_SIZE );

				if constexpr ( std::is_scalar_v< T > )
				{
					switch ( byte_count )
					{
						default:
							throw std::runtime_error( "Unknown size" );
						case 1:
							for ( std::size_t i = 0; i < accessor.count; ++i )
							{
								std::uint8_t tmp {};
								std::memcpy(
									&tmp,
									buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset
										+ ( i * byte_count ),
									byte_count );
								data.emplace_back( static_cast< T >( tmp ) );
							}
							return data;
						case 2:
							for ( std::size_t i = 0; i < accessor.count; ++i )
							{
								std::uint16_t tmp {};
								std::memcpy(
									&tmp,
									buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset
										+ ( i * byte_count ),
									byte_count );
								data.emplace_back( static_cast< T >( tmp ) );
							}
							return data;
						case 4:
							for ( std::size_t i = 0; i < accessor.count; ++i )
							{
								std::uint32_t tmp {};
								std::memcpy(
									&tmp,
									buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset
										+ ( i * byte_count ),
									byte_count );
								data.emplace_back( static_cast< T >( tmp ) );
							}
							return data;
						case 8:
							for ( std::size_t i = 0; i < accessor.count; ++i )
							{
								std::uint64_t tmp {};
								std::memcpy(
									&tmp,
									buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset
										+ ( i * byte_count ),
									byte_count );
								data.emplace_back( static_cast< T >( tmp ) );
							}
							return data;
					}
				}
				else
				{
					throw std::runtime_error(
						std::format( "Tried extracting data of size {} into type of size {}", byte_count, T_SIZE ) );
				}
			}
			else
			{
				throw std::runtime_error(
					std::format( "Tried extracting data of size {} into type of size {}", byte_count, T_SIZE ) );
			}
		}

		// Size matches perfectly. We can copy the data directly
		const auto real_size { byte_count * accessor.count };

		data.resize( accessor.count );

		std::memcpy( data.data(), buffer.data.data() + buffer_view.byteOffset + accessor.byteOffset, real_size );

		return data;
	}

	std::vector< std::uint32_t > SceneBuilder::
		extractIndicies( const tinygltf::Primitive& prim, const tinygltf::Model& model )
	{
		ZoneScoped;
		const auto& indicies_accessor { model.accessors.at( prim.indices ) };

		if ( indicies_accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT )
		{
			return extractData< std::uint32_t >( model, indicies_accessor );
		}
		else
		{
			//TODO: Figure out any time we can use a smaller indicies value instead of a 32 bit number all the time
			std::vector< std::uint32_t > indicies {};

			const auto tmp { extractData< std::uint32_t >( model, indicies_accessor ) };

			indicies.reserve( tmp.size() );

			for ( const auto val : tmp ) indicies.emplace_back( val );

			return indicies;
		}
	}

	const tinygltf::Accessor& SceneBuilder::getAccessorForAttribute(
		const tinygltf::Primitive& prim, const tinygltf::Model& root, const std::string attrib ) const
	{
		ZoneScoped;
		return root.accessors.at( prim.attributes.at( attrib ) );
	}

	std::shared_ptr< Texture > SceneBuilder::
		getTextureForParameter( const tinygltf::Parameter& parameter, const tinygltf::Model& root )
	{
		const auto texture_idx { parameter.TextureIndex() };
		return loadTexture( texture_idx, root );
	}

	PrimitiveTextures SceneBuilder::loadTextures( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		const auto mat_idx { prim.material };
		if ( mat_idx == -1 )
		{
			//There is no textures for this material
			// throw std::runtime_error( "No material for primitive. One was expected" );
			return {};
		}

		const tinygltf::Material& materials { root.materials[ mat_idx ] };

		auto findParameter = [ &materials ]( const std::string name ) -> std::optional< tinygltf::Parameter >
		{
			const auto& itter { materials.values.find( name ) };

			if ( itter == materials.values.end() )
				return std::nullopt;
			else
				return { itter->second };
		};

		std::string str {};

		std::size_t counter { 0 };

		for ( const auto& [ name, parameter ] : materials.values )
		{
			str += name;
			if ( counter + 1 != materials.values.size() ) str += ", ";
			++counter;
		}

		PrimitiveTextures textures {};

		const auto albedo { findParameter( "baseColorTexture" ) };
		if ( albedo.has_value() )
		{
			textures.albedo = getTextureForParameter( *albedo, root );
		}

		const auto normal { findParameter( "normalTexture" ) };
		if ( normal.has_value() )
		{
			textures.normal = getTextureForParameter( *normal, root );
		}

		const auto occlusion_texture { findParameter( "occlusionTexture" ) };

		const auto mettalic_roughness_texture { findParameter( "mettalicRoughnessTexture" ) };
		if ( mettalic_roughness_texture.has_value() )
		{
			textures.metallic_roughness = getTextureForParameter( *mettalic_roughness_texture, root );
		}

		return textures;
	}

	std::vector< GameObject > SceneBuilder::getGameObjects()
	{
		std::vector< GameObject > objects { std::move( this->game_objects ) };

		return objects;
	}

	std::vector< glm::vec3 > SceneBuilder::
		extractPositionInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		const tinygltf::Accessor& accessor { getAccessorForAttribute( prim, root, "POSITION" ) };

		return extractData< glm::vec3 >( root, accessor );
	}

	std::vector< glm::vec3 > SceneBuilder::
		extractNormalInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		if ( !hasAttribute( prim, "NORMAL" ) ) return {};
		const tinygltf::Accessor& accessor { getAccessorForAttribute( prim, root, "NORMAL" ) };

		return extractData< glm::vec3 >( root, accessor );
	}

	std::vector< glm::vec2 > SceneBuilder::extractUVInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		//TODO: Figure out how I can use multiple textures for various things.
		if ( !hasAttribute( prim, "TEXCOORD_0" ) ) return {};

		const tinygltf::Accessor& accessor { getAccessorForAttribute( prim, root, "TEXCOORD_0" ) };

		return extractData< glm::vec2 >( root, accessor );
	}

	bool SceneBuilder::hasAttribute( const tinygltf::Primitive& prim, const std::string_view str )
	{
		ZoneScoped;
		return prim.attributes.contains( std::string( str ) );
	}

	std::vector< ModelVertex > SceneBuilder::
		extractVertexInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		const auto pos { extractPositionInfo( prim, root ) };

		std::vector< ModelVertex > verts {};

		verts.reserve( pos.size() );

		const std::vector< glm::vec3 > normals { extractNormalInfo( prim, root ) };

		//TODO: If we don't have normals we likely will need to compute them ourselves.
		// I have no idea if this is actually going to be needed for us. But I might wanna implement it
		// anyways, Just in case.
		const bool has_normals { !normals.empty() };

		const std::vector< glm::vec2 > uvs { extractUVInfo( prim, root ) };
		const bool has_uv { !uvs.empty() };

		for ( std::size_t i = 0; i < pos.size(); ++i )
		{
			ModelVertex vert {};
			vert.m_position = pos[ i ];
			vert.m_normal = has_normals ? normals[ i ] : glm::vec3();
			vert.m_uv = has_uv ? uvs[ i ] : glm::vec2();
			vert.m_color = glm::vec3( 0.1f );
			verts.emplace_back( vert );
		}

		return verts;
	}

	Primitive SceneBuilder::loadPrimitive( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		ZoneScoped;
		std::string att_str { "" };
		for ( const auto& attrib : prim.attributes )
		{
			att_str += attrib.first + ", ";
		}

		//TODO: Get normal colors from texture
		[[maybe_unused]] const bool has_normal { hasAttribute( prim, "NORMAL" ) };
		const bool has_position { hasAttribute( prim, "POSITION" ) };
		const bool has_texcoord { hasAttribute( prim, "TEXCOORD_0" ) };
		[[maybe_unused]] const int texcoord_count { has_texcoord ? getTexcoordCount( prim ) : 0 };

		if ( !has_position ) throw std::runtime_error( "Failed to load model. Missing expected POSITION attribute" );

		switch ( static_cast< PrimitiveMode >( prim.mode ) )
		{
			case POINTS:
				[[fallthrough]];
			case LINE:
				[[fallthrough]];
			case LINE_LOOP:
				[[fallthrough]];
			case LINE_STRIP:
				[[fallthrough]];
			case TRIS:
				[[fallthrough]];
			case TRI_STRIP:
				[[fallthrough]];
			case TRI_FAN:
				{
					std::vector< ModelVertex > verts { extractVertexInfo( prim, root ) };
					std::vector< std::uint32_t > indicies { extractIndicies( prim, root ) };

					Primitive primitive_mesh { Primitive::fromVerts(
						std::move( verts ),
						static_cast< PrimitiveMode >( prim.mode ),
						std::move( indicies ),
						m_vertex_buffer,
						m_index_buffer ) };

					// If we have a texcoord then we have a UV map. Meaning we likely have textures to use
					if ( !has_texcoord ) return primitive_mesh;

					//primitive_mesh.m_textures = loadTextures( prim, root );
					primitive_mesh.m_material = loadMaterial( prim, root );

					return primitive_mesh;
				}
			default:
				{
					log::error( "Unsupported mode for primtiive loading: {}", prim.mode );
					throw std::runtime_error( "Unsupported mode for primitive loading" );
				}
		}

		FGL_UNREACHABLE();
	}

	OrientedBoundingBox< CoordinateSpace::Model > createModelBoundingBox( const std::vector< Primitive >& primitives )
	{
		ZoneScoped;
		if ( primitives.size() <= 0 ) return {};

		OrientedBoundingBox< CoordinateSpace::Model > box { primitives.at( 0 ).m_bounding_box };

		for ( std::uint64_t i = 1; i < primitives.size(); i++ ) box = box.combine( primitives[ i ].m_bounding_box );

		return box;
	}

	glm::vec3 convertToVec3( const std::vector< double >& data )
	{
		return { static_cast< float >( data[ 0 ] ),
			     static_cast< float >( data[ 1 ] ),
			     static_cast< float >( data[ 2 ] ) };
	}

	glm::vec4 convertToVec4( const std::vector< double >& data )
	{
		return { static_cast< float >( data[ 0 ] ),
			     static_cast< float >( data[ 1 ] ),
			     static_cast< float >( data[ 2 ] ),
			     static_cast< float >( data[ 3 ] ) };
	}

	WorldTransform SceneBuilder::loadTransform( int node_idx, const tinygltf::Model& root )
	{
		const auto node { root.nodes[ node_idx ] };

		FGL_ASSERT( node.matrix.size() == 0, "No matrix handler in loadTransform" );

		WorldTransform transform_component {};

		if ( node.rotation.size() == 4 )
		{
			const auto& x { node.rotation[ 0 ] };
			const auto& y { node.rotation[ 1 ] };
			const auto& z { node.rotation[ 2 ] };
			const auto& w { node.rotation[ 3 ] };

			// Quat is stored as (x,y,z,w) in gltf
			const glm::quat rotation { static_cast< float >( w ),
				                       static_cast< float >( x ),
				                       static_cast< float >( y ),
				                       static_cast< float >( z ) };

			transform_component.rotation = UniversalRotation( rotation );
		}
		else if ( !node.rotation.empty() )
		{
			log::warn( "Invalid rotation size: {}", node.rotation.size() );
			throw std::runtime_error( "Invalid rotation size" );
		}

		if ( node.scale.size() == 3 )
		{
			const glm::vec3 scale { convertToVec3( node.scale ) };
			transform_component.scale = scale;
		}
		else if ( !node.scale.empty() )
		{
			log::warn( "Odd scale size: {}", node.scale.size() );
		}

		if ( node.translation.size() == 3 )
		{
			const glm::vec3 translation { convertToVec3( node.translation ) };
			transform_component.translation = WorldCoordinate( translation );
		}
		else if ( !node.translation.empty() )
		{
			log::warn( "Odd translation size: {}", node.translation.size() );
		}

		return transform_component;
	}

	std::shared_ptr< Model > SceneBuilder::loadModel( const int mesh_idx, const tinygltf::Model& root )
	{
		ZoneScoped;
		const auto mesh { root.meshes[ mesh_idx ] };
		const auto& primitives { mesh.primitives };

		std::vector< Primitive > finished_primitives {};

		for ( const auto& prim : primitives )
		{
			Primitive primitive { loadPrimitive( prim, root ) };
			finished_primitives.emplace_back( std::move( primitive ) );
		}

		const auto bounding_box { createModelBoundingBox( finished_primitives ) };

		return std::make_shared<
			Model >( std::move( finished_primitives ), bounding_box, mesh.name.empty() ? "Unnamed Model" : mesh.name );
	}

	std::shared_ptr< Texture > SceneBuilder::loadTexture( const int tex_id, const tinygltf::Model& root ) const
	{
		if ( tex_id == -1 ) return { nullptr };

		const tinygltf::Texture& tex_info { root.textures[ tex_id ] };

		const auto source_idx { tex_info.source };

		const tinygltf::Image& source { root.images[ source_idx ] };

		if ( source.uri.empty() ) throw std::runtime_error( "Unsupported loading method for image (Must be a file)" );

		const std::filesystem::path filepath { source.uri };
		const auto full_path { m_root / filepath };

		const auto sampler_idx { tex_info.sampler };
		const tinygltf::Sampler& sampler_info { root.samplers[ sampler_idx ] };

		Sampler sampler { sampler_info.minFilter, sampler_info.magFilter, sampler_info.wrapS, sampler_info.wrapT };

		std::shared_ptr< Texture > texture { getTextureStore().load( full_path ) };
		texture->getImageView().getSampler() = std::move( sampler );

		//Prepare the texture into the global system
		Texture::getDescriptorSet().bindTexture( 0, texture );
		Texture::getDescriptorSet().update();

		return texture;
	}

	std::shared_ptr< Material > SceneBuilder::
		loadMaterial( const tinygltf::Primitive& prim, const tinygltf::Model& root )
	{
		const auto& material_id { prim.material };
		const auto& gltf_material { root.materials[ material_id ] };

		auto material { Material::createMaterial() };

		{
			const auto& metallic_roughness { gltf_material.pbrMetallicRoughness };
			const auto& pbr_tex_id { metallic_roughness.baseColorTexture.index };
			material->properties.pbr.color_tex = loadTexture( pbr_tex_id, root );

			material->properties.pbr.color_factors = { metallic_roughness.baseColorFactor[ 0 ],
				                                       metallic_roughness.baseColorFactor[ 1 ],
				                                       metallic_roughness.baseColorFactor[ 2 ],
				                                       metallic_roughness.baseColorFactor[ 3 ] };

			material->properties.pbr.metallic_roughness_tex =
				loadTexture( metallic_roughness.metallicRoughnessTexture.index, root );
			material->properties.pbr.metallic_factor = metallic_roughness.metallicFactor;
			material->properties.pbr.roughness_factor = metallic_roughness.roughnessFactor;
		}

		material->properties.normal.texture = loadTexture( gltf_material.normalTexture.index, root );
		material->properties.normal.scale = gltf_material.normalTexture.scale;

		material->properties.occlusion.texture = loadTexture( gltf_material.occlusionTexture.index, root );
		material->properties.occlusion.strength = gltf_material.occlusionTexture.strength;

		material->properties.emissive.texture = loadTexture( gltf_material.emissiveTexture.index, root );
		material->properties.emissive.factors = convertToVec3( gltf_material.emissiveFactor );

		material->update();

		return material;
	}

	void SceneBuilder::handleNode( const int node_idx, const tinygltf::Model& root )
	{
		ZoneScoped;
		const tinygltf::Node& node { root.nodes[ node_idx ] };

		const int mesh_idx { node.mesh };
		const int skin_idx { node.skin };

		GameObject obj { GameObject::createGameObject() };

		std::shared_ptr< Model > model { loadModel( mesh_idx, root ) };

		assert( model );

		std::unique_ptr< components::ModelComponent > component {
			std::make_unique< components::ModelComponent >( std::move( model ) )
		};

		obj.addComponent( std::move( component ) );

		obj.addFlag( IsVisible | IsEntity );

		const auto transform { loadTransform( node_idx, root ) };

		obj.getTransform() = transform;

		if ( node.name.empty() )
			obj.setName( "Unnamed Game Object" );
		else
			obj.setName( node.name );

		this->game_objects.emplace_back( std::move( obj ) );
	}

	void SceneBuilder::handleScene( const tinygltf::Scene& scene, const tinygltf::Model& root )
	{
		ZoneScoped;
		for ( const auto node_idx : scene.nodes )
		{
			handleNode( node_idx, root );
		}
	}

	void SceneBuilder::loadScene( const std::filesystem::path& path )
	{
		ZoneScoped;
		if ( !std::filesystem::exists( path ) ) throw std::runtime_error( "Failed to find scene at filepath" );
		m_root = path.parent_path();

		tinygltf::TinyGLTF loader {};
		tinygltf::Model gltf_model {};

		std::string err {};
		std::string warn {};

		if ( path.extension() == ".gltf" )
		{
			//Must use the ASCII loader
			if ( !loader.LoadASCIIFromFile( &gltf_model, &err, &warn, path ) )
			{
				log::info( "Failed to scene at {}", path );
				if ( !warn.empty() ) log::warn( warn );
				if ( !err.empty() ) log::error( err );
				throw std::runtime_error( "Failed to load binary model" );
			}
		}
		else
		{
			if ( !loader.LoadBinaryFromFile( &gltf_model, &err, &warn, path ) )
			{
				log::info( "Failed to scene at {}", path );
				if ( !warn.empty() ) log::warn( warn );
				if ( !err.empty() ) log::error( err );
				throw std::runtime_error( "Failed to load binary model" );
			}
		}

		if ( !err.empty() )
		{
			log::error( "Error loading model {}: \"{}\"", path.string(), err );
			throw std::runtime_error( err );
		}

		if ( !warn.empty() )
		{
			log::warn( "Warning loading model {}: \"{}\"", path.string(), warn );
		}

		const auto scenes { gltf_model.scenes };

		for ( const auto& scene : scenes )
		{
			handleScene( scene, gltf_model );
		}
	}

} // namespace fgl::engine
