//
// Created by kj16609 on 3/1/24.
//

#include "OctTreeNode.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include "engine/debug/drawers.hpp"
#include "engine/primitives/Frustum.hpp"

namespace fgl::engine
{

	std::vector< NodeLeaf* > OctTreeNode::getAllLeafsInFrustum( const Frustum< CoordinateSpace::World >& frustum )
	{
		ZoneScoped;
		std::vector< NodeLeaf* > leafs {};

		//Check if we are inside of the frustum.
		if ( isInFrustum( frustum ) )
		{
			switch ( m_node_data.index() )
			{
				case 0: // NodeArray
					{
						assert( std::holds_alternative< NodeArray >( m_node_data ) );
						NodeArray& node_array { std::get< NodeArray >( m_node_data ) };
						//Search deeper

#ifndef NDEBUG
						for ( int x = 0; x < 2; ++x )
							for ( int y = 0; y < 2; ++y )
								for ( int z = 0; z < 2; ++z ) assert( node_array[ x ][ y ][ z ] );
#endif

						{
							const auto ret { node_array[ LEFT ][ FORWARD ][ TOP ]->getAllLeafsInFrustum( frustum ) };
							leafs = std::move( ret );
						}
						{
							const auto ret { node_array[ LEFT ][ FORWARD ][ BOTTOM ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						{
							const auto ret { node_array[ LEFT ][ BACK ][ TOP ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						{
							const auto ret { node_array[ LEFT ][ BACK ][ BOTTOM ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}

						{
							const auto ret { node_array[ RIGHT ][ FORWARD ][ TOP ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						{
							const auto ret {
								node_array[ RIGHT ][ FORWARD ][ BOTTOM ]->getAllLeafsInFrustum( frustum )
							};
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						{
							const auto ret { node_array[ RIGHT ][ BACK ][ TOP ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						{
							const auto ret { node_array[ RIGHT ][ BACK ][ BOTTOM ]->getAllLeafsInFrustum( frustum ) };
							leafs.insert( leafs.end(), ret.begin(), ret.end() );
						}
						return leafs;
					}
				case 1: // NodeLeaf
					{
						assert( std::holds_alternative< NodeLeaf >( m_node_data ) );
						leafs.reserve( 4096 );
						leafs.emplace_back( &std::get< NodeLeaf >( m_node_data ) );

						//debug::world::drawBoundingBox( m_bounds );

						return leafs;
					}
				default:
					throw std::runtime_error( "OctTreeNode::Index out of bounds" );
			}
		}

		return leafs;
	}

	OctTreeNode::OctTreeNode( const WorldCoordinate center, float span, OctTreeNode* parent ) :
	  m_bounds( center, span ),
	  m_node_data( NodeLeaf() ),
	  m_parent( parent )
	{
		assert( std::holds_alternative< NodeLeaf >( m_node_data ) );
		std::get< NodeLeaf >( m_node_data ).reserve( MAX_NODES_IN_LEAF );
	}

	void OctTreeNode::split( int depth )
	{
		if ( std::holds_alternative< NodeArray >( m_node_data ) ) return;
		auto& game_objects { std::get< NodeLeaf >( m_node_data ) };

		//Figure out the half span
		const float half_span { m_bounds.span() / 2.0f };

		const Coordinate< CoordinateSpace::World > center { m_bounds.getPosition() };

		NodeArray new_nodes {};

		const float left_x { center.x - half_span };
		const float right_x { center.x + half_span };

		const float forward_y { center.y - half_span };
		const float backward_y { center.y + half_span };

		const float top_z { center.z - half_span };
		const float bottom_z { center.z + half_span };

		new_nodes[ LEFT ][ FORWARD ][ TOP ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, forward_y, top_z ), half_span, this );

		new_nodes[ LEFT ][ FORWARD ][ BOTTOM ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, forward_y, bottom_z ), half_span, this );

		new_nodes[ LEFT ][ BACK ][ TOP ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, backward_y, top_z ), half_span, this );

		new_nodes[ LEFT ][ BACK ][ BOTTOM ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( left_x, backward_y, bottom_z ), half_span, this );

		new_nodes[ RIGHT ][ FORWARD ][ TOP ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, forward_y, top_z ), half_span, this );

		new_nodes[ RIGHT ][ FORWARD ][ BOTTOM ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, forward_y, bottom_z ), half_span, this );

		new_nodes[ RIGHT ][ BACK ][ TOP ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, backward_y, top_z ), half_span, this );

		new_nodes[ RIGHT ][ BACK ][ BOTTOM ] =
			std::make_unique< OctTreeNode >( WorldCoordinate( right_x, backward_y, bottom_z ), half_span, this );

		for ( GameObject& obj : game_objects )
		{
			const auto& obj_coordinate { obj.m_transform.translation };
			const bool is_right { obj_coordinate.x > center.x };
			const bool is_forward { obj_coordinate.y > center.y };
			const bool is_up { obj_coordinate.z > center.z };

			std::unique_ptr< OctTreeNode >& node { new_nodes[ is_right ][ is_forward ][ is_up ] };
			assert( std::holds_alternative< NodeLeaf >( node->m_node_data ) );

			std::get< NodeLeaf >( node->m_node_data ).emplace_back( std::move( obj ) );
		}

		this->m_node_data = std::move( new_nodes );

		if ( depth - 1 >= 1 )
		{
			//TODO: Split all nodes again
		}
	}

	OctTreeNode* OctTreeNode::addGameObject( GameObject&& obj )
	{
		if ( std::holds_alternative< NodeLeaf >( m_node_data ) )
		{
			auto& objects { std::get< NodeLeaf >( m_node_data ) };
			assert( objects.capacity() == MAX_NODES_IN_LEAF );
			if ( objects.size() + 1 >= MAX_NODES_IN_LEAF )
			{
				split();
				return this->addGameObject( std::move( obj ) );
			}
			else
			{
				objects.emplace_back( std::move( obj ) );
				return this;
			}
		}
		else
		{
			const auto& center { m_bounds.getPosition() };
			const auto& obj_coordinate { obj.m_transform.translation };
			const bool is_right { obj_coordinate.x > center.x };
			const bool is_forward { obj_coordinate.y > center.y };
			const bool is_up { obj_coordinate.z > center.z };

			auto& nodes { std::get< NodeArray >( m_node_data ) };

			return nodes[ is_right ][ is_forward ][ is_up ]->addGameObject( std::move( obj ) );
		}
	}

	bool OctTreeNode::isInFrustum( const Frustum< CoordinateSpace::World >& frustum )
	{
		return !isEmpty() && frustum.intersects( m_bounds );
	}

	OctTreeNode* OctTreeNode::findID( const GameObject::ID id )
	{
		ZoneScoped;
		if ( std::holds_alternative< NodeLeaf >( this->m_node_data ) )
		{
			//We are the last node. Check if we have the ID
			const auto& game_objects { std::get< NodeLeaf >( m_node_data ) };

			if ( std::find_if(
					 game_objects.begin(),
					 game_objects.end(),
					 [ id ]( const GameObject& obj ) { return obj.m_id == id; } )
			     != game_objects.end() )
			{
				return this;
			}
			else
			{
				return nullptr;
			}
		}
		else if ( std::holds_alternative< NodeArray >( this->m_node_data ) )
		{
			const auto& node_array { std::get< NodeArray >( this->m_node_data ) };

			for ( std::size_t x = 0; x < 2; ++x )
			{
				for ( std::size_t y = 0; y < 2; ++y )
				{
					for ( std::size_t z = 0; z < 2; ++z )
					{
						const auto& node { node_array[ x ][ y ][ z ]->findID( id ) };
						if ( node != nullptr ) return node;
					}
				}
			}

			return nullptr;
		}

		std::unreachable();
	}

	auto OctTreeNode::getGameObjectItter( const GameObject::ID id )
	{
		assert( std::holds_alternative< NodeLeaf >( this->m_node_data ) );
		auto& game_objects { std::get< NodeLeaf >( this->m_node_data ) };
		return std::find_if(
			game_objects.begin(), game_objects.end(), [ id ]( const GameObject& obj ) { return id == obj.getId(); } );
	}

	bool OctTreeNode::canContain( const fgl::engine::GameObject& obj )
	{
		return m_bounds.contains( obj.m_transform.translation );
	}

	GameObject OctTreeNode::extract( const GameObject::ID id )
	{
		auto itter { getGameObjectItter( id ) };
		auto game_object { std::move( *itter ) };
		auto& game_objects { std::get< NodeLeaf >( this->m_node_data ) };
		game_objects.erase( itter );
		return game_object;
	}

	OctTreeNode* OctTreeNode::getRoot()
	{
		if ( m_parent == nullptr )
			return this;
		else
			return m_parent->getRoot();
	}

	std::vector< NodeLeaf* > OctTreeNode::getAllLeafs()
	{
		ZoneScoped;
		std::vector< NodeLeaf* > objects {};

		if ( std::holds_alternative< NodeLeaf >( m_node_data ) )
		{
			auto& leaf { std::get< NodeLeaf >( m_node_data ) };
			//No point in us giving back an empy leaf
			if ( leaf.size() > 0 ) objects.emplace_back( &leaf );
		}
		else
		{
			auto& nodes { std::get< NodeArray >( m_node_data ) };

			for ( std::size_t x = 0; x < 2; ++x )
			{
				for ( std::size_t y = 0; y < 2; ++y )
				{
					for ( std::size_t z = 0; z < 2; ++z )
					{
						auto ret { nodes[ x ][ y ][ z ]->getAllLeafs() };
						objects.insert( objects.end(), ret.begin(), ret.end() );
					}
				}
			}
		}

		return objects;
	}

} // namespace fgl::engine