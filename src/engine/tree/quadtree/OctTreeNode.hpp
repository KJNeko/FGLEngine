//
// Created by kj16609 on 3/1/24.
//

#pragma once

#include "engine/GameObject.hpp"
#include "engine/primitives/boxes/AxisAlignedBoundingCube.hpp"

namespace fgl::engine
{
	constexpr std::size_t AUTO_SPLIT_THRESHOLD { 512 };
	constexpr std::size_t STARTING_DEPTH { 4 };
	constexpr float ROOT_SPAN { 1000.0f };

	template < CoordinateSpace CType >
	struct Frustum;

	constexpr std::uint8_t TOP { 0 };
	constexpr std::uint8_t BOTTOM { 1 };
	constexpr std::uint8_t LEFT { 0 };
	constexpr std::uint8_t RIGHT { 1 };
	constexpr std::uint8_t FORWARD { 0 };
	constexpr std::uint8_t BACK { 1 };

	class OctTreeNode
	{
		using NodeArray = std::array< std::array< std::array< std::unique_ptr< OctTreeNode >, 2 >, 2 >, 2 >;
		using NodeLeaf = std::vector< GameObject >;

		AxisAlignedBoundingCube< CoordinateSpace::World > m_bounds;
		std::variant< NodeArray, NodeLeaf > m_node_data;

		OctTreeNode* m_parent;

	  public:

		OctTreeNode() = delete;

		//! Creates a root node
		//TODO: Lock behind a static explicit
		/**
		 * @param center
		 * @param span Distance from the center to any side
		 */
		OctTreeNode( const WorldCoordinate center, float span = ROOT_SPAN );

		OctTreeNode( const WorldCoordinate center, float span, std::vector< GameObject > objects );

		//! Splits a node. Does nothing if node is not a leaf.
		void split();

		bool isInFrustum( const Frustum< CoordinateSpace::World >& frustum );

		std::vector< GameObject > getAllInFrustum( const Frustum< CoordinateSpace::World >& frustum );

		//! Adds a game object, Will split the node if the auto split threshold is reached
		void addGameObject( const GameObject obj );
	};

} // namespace fgl::engine