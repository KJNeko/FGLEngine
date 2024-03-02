//
// Created by kj16609 on 3/1/24.
//

#include "OctTreeNode.hpp"

#include "engine/primitives/Frustum.hpp"

namespace fgl::engine
{

	std::vector< GameObject > OctTreeNode::getAllInFrustum( const Frustum< CoordinateSpace::World >& frustum )
	{}

	OctTreeNode::OctTreeNode( const WorldCoordinate center, float span ) :
	  m_bounds( center, span ),
	  m_node_data( NodeLeaf() ),
	  m_parent( nullptr )
	{}

	OctTreeNode::OctTreeNode( const WorldCoordinate center, float span, std::vector< GameObject > objects )
	{}

	void OctTreeNode::split()
	{
		//Figure out the half span
		const float half_span { m_bounds.span() / 2.0f };

		const Coordinate< CoordinateSpace::World > center { m_bounds.getPosition() };

		NodeArray new_nodes {};

		const float left_x { center.right() - half_span };
		const float right_x { center.right() + half_span };

		const float forward_y { center.forward() - half_span };
		const float backward_y { center.forward() + half_span };

		const float top_z { center.up() - half_span };
		const float bottom_z { center.up() + half_span };

		new_nodes[ TOP ][ FORWARD ][ LEFT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, forward_y, top_z ), half_span );
		new_nodes[ TOP ][ FORWARD ][ RIGHT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, forward_y, top_z ), half_span );
		new_nodes[ TOP ][ BACK ][ LEFT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, backward_y, top_z ), half_span );
		new_nodes[ TOP ][ BACK ][ RIGHT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, backward_y, top_z ), half_span );

		new_nodes[ BOTTOM ][ FORWARD ][ LEFT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, forward_y, bottom_z ), half_span );
		new_nodes[ BOTTOM ][ FORWARD ][ RIGHT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, forward_y, bottom_z ), half_span );
		new_nodes[ BOTTOM ][ BACK ][ LEFT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, backward_y, bottom_z ), half_span );
		new_nodes[ BOTTOM ][ BACK ][ RIGHT ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, backward_y, bottom_z ), half_span );

		this->m_node_data = std::move( new_nodes );
	}

} // namespace fgl::engine