//
// Created by kj16609 on 3/11/24.
//

#pragma once

#include "engine/GameObject.hpp"

namespace fgl::engine
{

	enum class CoordinateSpace;
	class QuadTreeNode;

	template < CoordinateSpace CType >
	struct Frusutm;

	using QuadTreeNodeArray = std::array< std::array< std::unique_ptr< QuadTreeNode >, 2 >, 2 >;
	using QuadTreeNodeLeaf = std::vector< GameObject >;

	constexpr std::size_t MAX_QUAD_NODES_IN_LEAF { 8 };

	class QuadTreeNode
	{
		WorldCoordinate m_center { constants::WORLD_CENTER };
		Scale m_node_bounds { std::numeric_limits< float >::infinity() };
		std::variant< QuadTreeNodeArray, QuadTreeNodeLeaf > m_node_data { QuadTreeNodeLeaf() };

		bool contains( const WorldCoordinate coord ) const;

	  public:

		QuadTreeNode& operator[]( const WorldCoordinate pos );

		void split( const int depth = 1 );

		void addGameObject( GameObject&& obj );
	};

} // namespace fgl::engine
