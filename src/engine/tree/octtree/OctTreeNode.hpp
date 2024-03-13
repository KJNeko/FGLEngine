//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include <bitset>
#include <functional>

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

	template < typename T >
	class OctTreeAllocator : public std::allocator< T >
	{
		std::vector< std::vector< std::byte > > blocks;
		using BlockIDX = int;

		//! Map for each pointer to their respective blocks
		std::unordered_map< T*, BlockIDX > m_block_map;
	};

	template < typename T >
	using unique_alloc_ptr = std::unique_ptr< T, std::function< void( T* ) > >;

	template < typename T, typename... Ts >
	unique_alloc_ptr< T > make_unique_from_allocator( OctTreeAllocator< T >& allocator, Ts... args )
	{
		T* ptr = allocator.allocate( 1 );
		allocator.construct( ptr, args... );

		auto deleter = [ &allocator ]( const auto* ptr_i ) -> void
		{
			allocator.destroy( ptr_i );
			allocator.deallocate( ptr_i, 1 );
		};

		return std::unique_ptr< T, decltype( deleter ) >( ptr, deleter );
	}

	using NodeArray = std::array< std::array< std::array< std::unique_ptr< OctTreeNode >, 2 >, 2 >, 2 >;
	using NodeLeaf = std::vector< GameObject >;

	static_assert( sizeof( NodeArray ) == sizeof( OctTreeNode* ) * 2 * 2 * 2 );
	static_assert( sizeof( OctTreeNode* ) == sizeof( std::uint64_t ) );

	struct FrameInfo;

	void imGuiOctTreeSettings( FrameInfo& info );

	class OctTreeNode
	{
		//! Fit to each model
		AxisAlignedBoundingBox< CoordinateSpace::World > m_fit_bounding_box;

		//! Real bounds of the node
		AxisAlignedBoundingCube< CoordinateSpace::World > m_bounds;

		using NodeDataT = NodeArray;
		using LeafDataT = NodeLeaf;

		std::variant< NodeDataT, LeafDataT > m_node_data;

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

		inline GameObject extract( const GameObject& obj ) { return this->extract( obj.getId() ); }

		bool isInFrustum( const Frustum< CoordinateSpace::World >& frustum ) const;

		bool isEmpty() const
		{
			return std::holds_alternative< NodeLeaf >( m_node_data ) && std::get< NodeLeaf >( m_node_data ).empty();
		}

		auto getGameObjectItter( const GameObject::ID id );

		void getAllLeafs( std::vector< NodeLeaf* >& out_leafs );
		void getAllLeafsInFrustum(
			const Frustum< CoordinateSpace::World >& frustum, std::vector< NodeLeaf* >& out_leafs );

	  public:

		//! Rebuilds the tree checking if nodes have moved.
		std::size_t reorganize();

		bool recalculateBoundingBoxes();

		constexpr static std::size_t LEAF_RESERVE_SIZE { 1024 };

		[[nodiscard]] inline std::vector< NodeLeaf* > getAllLeafs()
		{
			ZoneScoped;
			std::vector< NodeLeaf* > leafs;
			leafs.reserve( LEAF_RESERVE_SIZE );
			this->getAllLeafs( leafs );
			return leafs;
		}

		[[nodiscard]] inline std::vector< NodeLeaf* > getAllLeafsInFrustum( const Frustum< CoordinateSpace::World >&
		                                                                        frustum )
		{
			ZoneScoped;
			std::vector< NodeLeaf* > leafs;
			leafs.reserve( LEAF_RESERVE_SIZE );
			this->getAllLeafsInFrustum( frustum, leafs );
			return leafs;
		}

		//! Adds a game object, Will split the node if the auto split threshold is reached
		OctTreeNode* addGameObject( GameObject&& obj );
	};

} // namespace fgl::engine