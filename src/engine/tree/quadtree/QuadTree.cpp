//
// Created by kj16609 on 3/11/24.
//

#include "QuadTree.hpp"

#include "engine/gameobjects/GameObject.hpp"

namespace fgl::engine
{
	bool QuadTreeNode::contains( const WorldCoordinate coord ) const
	{
		const auto centered_coordinate { coord - m_center };

		const bool top_in_range { glm::all( glm::lessThanEqual( centered_coordinate.vec(), m_node_bounds ) ) };
		const bool bottom_in_range { glm::all( glm::greaterThanEqual( centered_coordinate.vec(), -m_node_bounds ) ) };
		return top_in_range && bottom_in_range;
	}

	QuadTreeNode& QuadTreeNode::operator[]( const WorldCoordinate pos )
	{
		assert( std::holds_alternative< QuadTreeNodeArray >( m_node_data ) );
		const auto test_dim { glm::greaterThanEqual( pos.vec(), this->m_center.vec() ) };

		auto& node_array { std::get< QuadTreeNodeArray >( m_node_data ) };
		const auto& node { node_array[ test_dim.x ][ test_dim.y ] };
		assert( node );

		return *node.get();
	}

	void QuadTreeNode::split( const int depth )
	{
		if ( std::holds_alternative< QuadTreeNodeArray >( m_node_data ) ) return;

		auto leaf_data { std::move( std::get< QuadTreeNodeLeaf >( m_node_data ) ) };

		QuadTreeNodeArray new_nodes {
			{ { { std::make_unique< QuadTreeNode >(), std::make_unique< QuadTreeNode >() } },
			  { { std::make_unique< QuadTreeNode >(), std::make_unique< QuadTreeNode >() } } }
		};

		m_node_data = std::move( new_nodes );

		[[assume( leaf_data.size() <= MAX_QUAD_NODES_IN_LEAF )]];

		for ( auto& item : leaf_data )
		{
			const auto item_pos { item.getPosition() };

			auto& node { ( *this )[ item_pos ] };
			assert( std::holds_alternative< QuadTreeNodeLeaf >( node.m_node_data ) );

			node.addGameObject( std::move( item ) );
		}

		if ( depth > 1 ) split( depth - 1 );
	}

	void QuadTreeNode::addGameObject( GameObject&& obj )
	{
		assert( contains( obj.getPosition() ) );

		if ( std::holds_alternative< QuadTreeNodeLeaf >( m_node_data ) )
		{
			std::get< QuadTreeNodeLeaf >( m_node_data ).emplace_back( std::forward< GameObject >( obj ) );
			return;
		}
		else
		{
			assert( std::holds_alternative< QuadTreeNodeArray >( m_node_data ) );
			( *this )[ obj.getPosition() ].addGameObject( std::forward< GameObject >( obj ) );
			return;
		}
	}

} // namespace fgl::engine
