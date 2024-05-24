//
// Created by kj16609 on 5/18/24.
//

#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <filesystem>
#include <vector>

namespace fgl::engine
{
	struct Vertex;
	class Model;
	struct Primitive;
	class Texture;
	class Buffer;
} // namespace fgl::engine

namespace tinygltf
{
	struct Scene;
	class Model;
	struct Primitive;
	struct Accessor;
} // namespace tinygltf

namespace fgl::engine
{
	class SceneBuilder
	{
		Buffer& m_vertex_buffer;
		Buffer& m_index_buffer;

		std::vector< std::shared_ptr< Model > > models {};

		void handleScene( const tinygltf::Scene& scene, const tinygltf::Model& root );
		void handleNode( const int node_idx, const tinygltf::Model& root );
		std::shared_ptr< Model > loadModel( const int mesh_idx, const tinygltf::Model& root );
		Primitive loadPrimitive( const tinygltf::Primitive& prim, const tinygltf::Model& model );

		int getTexcoordCount( const tinygltf::Primitive& prim ) const;

		std::vector< std::uint32_t > extractIndicies( const tinygltf::Primitive& prim, const tinygltf::Model& model );

		std::vector< Vertex > extractVertexInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		std::vector< glm::vec3 > extractPositionInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		//! Returns an empty vector if no normals could be extracted
		std::vector< glm::vec3 > extractNormalInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		std::vector< glm::vec2 > extractUVInfo( const tinygltf::Primitive& prim, const tinygltf::Model& root );

		bool hasAttribute( const tinygltf::Primitive& prim, const std::string_view str );

		const tinygltf::Accessor& getAccessorForAttribute(
			const tinygltf::Primitive& prim, const tinygltf::Model& root, const std::string attrib ) const;

		Texture loadTexture( const tinygltf::Primitive& prim, const tinygltf::Model& root );

	  public:

		std::vector< std::shared_ptr< Model > > getModels();

		SceneBuilder() = delete;

		SceneBuilder( Buffer& vertex_buffer, Buffer& index_buffer );

		void loadScene( const std::filesystem::path path );
	};

} // namespace fgl::engine