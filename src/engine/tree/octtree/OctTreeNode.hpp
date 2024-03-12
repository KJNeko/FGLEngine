//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include <bitset>

#include "engine/GameObject.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"

namespace fgl::engine
{
	constexpr std::size_t MAX_NODES_IN_LEAF { 32 };
	constexpr std::size_t STARTING_DEPTH { 4 };
	constexpr float ROOT_SPAN { std::numeric_limits< float >::max() };

	template < CoordinateSpace CType >
	struct Frustum;

	constexpr std::uint8_t TOP { 0 };
	constexpr std::uint8_t BOTTOM { 1 };
	constexpr std::uint8_t LEFT { 0 };
	constexpr std::uint8_t RIGHT { 1 };
	constexpr std::uint8_t FORWARD { 0 };
	constexpr std::uint8_t BACK { 1 };

	class OctTreeNode;
	class GameObject;

	using NodeArray = std::array< std::array< std::array< std::unique_ptr< OctTreeNode >, 2 >, 2 >, 2 >;
	using NodeLeaf = std::vector< GameObject >;

	static_assert( sizeof( NodeArray ) == sizeof( OctTreeNode* ) * 2 * 2 * 2 );
	static_assert( sizeof( OctTreeNode* ) == sizeof( std::uint64_t ) );

	void imGuiOctTreeSettings();

	class OctTreeNode
	{
		//! Fit to each model
		AxisAlignedBoundingBox< CoordinateSpace::World > m_fit_bounding_box;

		//! Real bounds of the node
		AxisAlignedBoundingCube< CoordinateSpace::World > m_bounds;

		std::variant< NodeArray, NodeLeaf > m_node_data;

		OctTreeNode* m_parent;

	  public:

		OctTreeNode() = delete;
		OctTreeNode( const WorldCoordinate center, float span = ROOT_SPAN, OctTreeNode* parent = nullptr );

		OctTreeNode( const OctTreeNode& other ) = delete;
		OctTreeNode( OctTreeNode&& other ) = delete;

		OctTreeNode& operator=( const OctTreeNode& ) = delete;
		OctTreeNode& operator=( OctTreeNode&& ) = delete;

	  private:

		//! Returns the node of a given ID (Searches down)
		OctTreeNode* findID( const GameObject::ID id );

		//! Returns true if the node contains a given ID
		inline bool contains( const GameObject::ID id ) { return findID( id ) != nullptr; }

		//! Splits a node. Does nothing if node is not a leaf.
		void split( int depth = 1 );

		OctTreeNode* getRoot();

		//! returns true if this node should contain the given object
		bool canContain( const GameObject& obj );

		GameObject extract( const GameObject::ID id );

		bool isInFrustum( const Frustum< CoordinateSpace::World >& frustum );

		bool isEmpty() const
		{
			return std::holds_alternative< NodeLeaf >( m_node_data ) && std::get< NodeLeaf >( m_node_data ).empty();
		}

		auto getGameObjectItter( const GameObject::ID id );

	  public:

		bool recalculateBoundingBoxes();

		std::vector< NodeLeaf* > getAllLeafs();

		std::vector< NodeLeaf* > getAllLeafsInFrustum( const Frustum< CoordinateSpace::World >& frustum );

		//! Adds a game object, Will split the node if the auto split threshold is reached
		OctTreeNode* addGameObject( GameObject&& obj );
	};

} // namespace fgl::engine