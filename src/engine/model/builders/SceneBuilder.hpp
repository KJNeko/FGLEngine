//
// Created by kj16609 on 5/18/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wduplicated-branches"
#include <glm/gtx/hash.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#pragma GCC diagnostic pop

#include <filesystem>
#include <vector>

#include "engine/gameobjects/GameObject.hpp"
#include "engine/primitives/TransformComponent.hpp"

namespace fgl::engine
{
	struct PrimitiveTextures;
	struct ModelVertex;
	class Model;
	struct Primitive;
	class Texture;

	namespace memory
	{
		class Buffer;
	}

} // namespace fgl::engine

namespace tinygltf
{
	struct Scene;
	class Model;
	struct Primitive;
	struct Accessor;
	struct Parameter;
} // namespace tinygltf

namespace fgl::engine
{
	class SceneBuilder
	{
		//! Root path. Set by 'load' functions
		std::filesystem::path m_root {};
		memory::Buffer& m_vertex_buffer;
		memory::Buffer& m_index_buffer;

		std::vector< GameObject > game_objects {};

		void handleScene( const tinygltf::Scene& scene, const tinygltf::Model& root );
		void handleNode( const int node_idx, const tinygltf::Model& root );

		TransformComponent loadTransform( int node_idx, const tinygltf::Model& root );
		std::shared_ptr< Model > loadModel( const int mesh_idx, const tinygltf::Model& root );
		Primitive loadPrimitive( const tinygltf::Primitive& prim, const tinygltf::Model& model );

		int getTexcoordCount( const tinygltf::Primitive& prim ) const;

		std::vector< std::uint32_t > extractIndicies( const tinygltf::Primitive& prim, const tinygltf::Model& model );

		std::vector< ModelVertex > extractVertexInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		std::vector< glm::vec3 > extractPositionInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		//! Returns an empty vector if no normals could be extracted
		std::vector< glm::vec3 > extractNormalInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		std::vector< glm::vec2 > extractUVInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		bool hasAttribute( const tinygltf::Primitive& prim, const std::string_view str );

		const tinygltf::Accessor& getAccessorForAttribute(
			const tinygltf::Primitive& prim, const tinygltf::Model& root, const std::string attrib ) const;
		std::shared_ptr< Texture >
			getTextureForParameter( const tinygltf::Parameter& parameter, const tinygltf::Model& root );

		PrimitiveTextures loadTextures( const tinygltf::Primitive& prim, const tinygltf::Model& root );

	  public:

		std::vector< GameObject > getGameObjects();

		SceneBuilder() = delete;

		SceneBuilder( memory::Buffer& vertex_buffer, memory::Buffer& index_buffer );

		void loadScene( const std::filesystem::path path );
	};

} // namespace fgl::engine