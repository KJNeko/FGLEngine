//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"
#include "gameobjects/GameObject.hpp"

namespace fgl::engine
{
	constexpr std::size_t MAX_NODES_IN_LEAF { 32 };
	constexpr std::size_t STARTING_DEPTH { 1 };
	constexpr float ROOT_SPAN { std::numeric_limits< float >::max() };

	struct Frustum;

	constexpr std::uint8_t TOP { 1 };
	constexpr std::uint8_t BOTTOM { 0 };

	constexpr std::uint8_t RIGHT { 1 };
	constexpr std::uint8_t LEFT { 0 };

	constexpr std::uint8_t FORWARD { 1 };
	constexpr std::uint8_t BACK { 0 };

	class OctTreeNode;
	class GameObject;

	using OctTreeNodeArray = std::array< std::array< std::array< std::unique_ptr< OctTreeNode >, 2 >, 2 >, 2 >;
	using OctTreeNodeLeaf = std::vector< GameObject >;

	static_assert( sizeof( OctTreeNodeArray ) == sizeof( OctTreeNode* ) * 2 * 2 * 2 );
	static_assert( sizeof( OctTreeNode* ) == sizeof( std::uint64_t ) );

	struct FrameInfo;

	void imGuiOctTreeSettings( const FrameInfo& info );

	class OctTreeNode
	{
		//! Fit to each model
		AxisAlignedBoundingBox< CoordinateSpace::World > m_fit_bounding_box;

		//! Real bounds of the node
		AxisAlignedBoundingCube< CoordinateSpace::World > m_bounds;

		using NodeDataT = OctTreeNodeArray;
		using LeafDataT = OctTreeNodeLeaf;

		std::variant< NodeDataT, LeafDataT > m_node_data;

		OctTreeNode* m_parent;

		//! Node to skip too when attempting to parse this node.
		//! This is done in order to prevent navigating through an empty set of nodes.
		//! This is only set if there is only one leaf filled with data.
		OctTreeNode* m_skip { nullptr };

	  public:

		OctTreeNode() = delete;
		OctTreeNode( WorldCoordinate center, float span = ROOT_SPAN, OctTreeNode* parent = nullptr );

		OctTreeNode( const OctTreeNode& other ) = delete;
		OctTreeNode( OctTreeNode&& other ) = delete;

		~OctTreeNode();

		OctTreeNode& operator=( const OctTreeNode& ) = delete;
		OctTreeNode& operator=( OctTreeNode&& ) = delete;
		void clear();
		WorldCoordinate getCenter() const;
		WorldCoordinate getFitCenter() const;
		void drawDebug() const;
		void recalculateChildBounds();

	  private:

		//! Returns the node of a given ID (Searches down)
		OctTreeNode* findID( GameObject::GameObjectID id );

		//! Returns true if the node contains a given ID
		bool contains( const GameObject::GameObjectID id ) { return findID( id ) != nullptr; }

		//! Splits a node. Does nothing if node is not a leaf.
		void split( int depth = 1 );

		OctTreeNode* getRoot();

		//! returns true if this node should contain the given object
		bool canContain( const GameObject& obj ) const;
		bool canContain( const WorldCoordinate& coord ) const;

		GameObject extract( GameObject::GameObjectID id );

		GameObject extract( const GameObject& obj ) { return this->extract( obj.getId() ); }

		bool isInFrustum( const Frustum& frustum ) const;

		bool isEmpty() const;

		auto getGameObjectItter( GameObject::GameObjectID id );

		void getAllLeafs( std::vector< OctTreeNodeLeaf* >& out_leafs );
		void getAllLeafsInFrustum( const Frustum& frustum, std::vector< OctTreeNodeLeaf* >& out_leafs );

		OctTreeNode& operator[]( const WorldCoordinate coord ) const;

	  public:

		//! Rebuilds the tree checking if nodes have moved.
		std::size_t reorganize();
		OctTreeNode* optimizePath();

		//! Returns true if the fixed bounding box is larger then the inital bounding box
		bool isBoundsExpanded() const;

		void recalculateNodeBounds();

		void recalculateLeafBounds();

		void recalculateBounds();

		constexpr static std::size_t LEAF_RESERVE_SIZE { 1024 };

		[[nodiscard]] std::vector< OctTreeNodeLeaf* > getAllLeafs();

		[[nodiscard]] std::vector< OctTreeNodeLeaf* > getAllLeafsInFrustum( const Frustum& frustum );

		//! Adds a game object, Will split the node if the auto split threshold is reached
		OctTreeNode* addGameObject( GameObject&& obj );
		bool isLeaf() const;
		bool isBranch() const;

		std::size_t itemCount() const;
		const OctTreeNodeArray& getBranches() const;
		const OctTreeNodeLeaf& getLeaf() const;
		OctTreeNodeLeaf& getLeaf();
	};

#define FOR_EACH_OCTTREE_NODE                                                                                          \
	for ( std::size_t x = 0; x < 2; ++x )                                                                              \
		for ( std::size_t y = 0; y < 2; ++y )                                                                          \
			for ( std::size_t z = 0; z < 2; ++z )

} // namespace fgl::engine