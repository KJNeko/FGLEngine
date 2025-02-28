//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include <memory>
#include <vector>

#include "engine/primitives/Scale.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	class GameObject;

	enum class CoordinateSpace;
	class QuadTreeNode;

	template < CoordinateSpace CType >
	struct Frustum;

	using QuadTreeNodeArray = std::array< std::array< std::unique_ptr< QuadTreeNode >, 2 >, 2 >;
	using QuadTreeNodeLeaf = std::vector< GameObject >;

	constexpr std::size_t MAX_QUAD_NODES_IN_LEAF { 8 };

	class QuadTreeNode
	{
		WorldCoordinate m_center { constants::WORLD_CENTER };
		Scale m_node_bounds { std::numeric_limits< float >::infinity() };
		std::variant< QuadTreeNodeArray, QuadTreeNodeLeaf > m_node_data { QuadTreeNodeLeaf() };

		bool contains( WorldCoordinate coord ) const;

	  public:

		QuadTreeNode& operator[]( WorldCoordinate pos );

		void split( int depth = 1 );

		void addGameObject( GameObject&& obj );
	};

} // namespace fgl::engine
