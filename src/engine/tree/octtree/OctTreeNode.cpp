//
// Created by kj16609 on 3/1/24.
//

#include "OctTreeNode.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <engine/FrameInfo.hpp>
#include <glm/gtx/string_cast.hpp>

#include "engine/debug/drawers.hpp"
#include "engine/model/Model.hpp"
#include "engine/primitives/Frustum.hpp"

namespace fgl::engine
{
	struct FrameInfo;

	static bool draw_leaf_fit_bounds { false };
	static bool draw_leaf_real_bounds { false };
	static bool draw_inview_bounds { false };
	static std::size_t number_moved { 0 };
	static std::optional< std::chrono::microseconds > time { std::nullopt };

	void imGuiOctTreeSettings( FrameInfo& info )
	{
		/*
#if ENABLE_IMGUI
		if ( ImGui::CollapsingHeader( "OctTree debug settings" ) )
		{
			ImGui::Checkbox( "Draw leaf fitted bounding boxes", &draw_leaf_fit_bounds );
			ImGui::Checkbox( "Draw leaf real bounding boxes", &draw_leaf_real_bounds );
			ImGui::Checkbox( "Draw ALL in view bounding boxes", &draw_inview_bounds );

			if ( ImGui::Button( "Reorganize Octtree" ) )
			{
				const auto start { std::chrono::high_resolution_clock::now() };
				number_moved = info.game_objects.reorganize();
				const auto end { std::chrono::high_resolution_clock::now() };
				const auto time_diff { end - start };
				time = std::chrono::duration_cast< std::chrono::microseconds >( time_diff );
			}

			if ( time.has_value() )
			{
				ImGui::Text( "Time spent reorganizing: %.2ldus", time.value().count() );
				ImGui::Text( "Moved %ld objects", number_moved );
			}
		}
#endif*/
	}

	void OctTreeNode::getAllLeafsInFrustum(
		const Frustum< CoordinateSpace::World >& frustum, std::vector< OctTreeNodeLeaf* >& out_leafs )
	{
		//Check if we are inside of the frustum.
		if ( !isInFrustum( frustum ) ) return;

		auto& leafs { out_leafs };

		switch ( m_node_data.index() )
		{
			case 0: // NodeArray
				{
					assert( std::holds_alternative< OctTreeNodeArray >( m_node_data ) );
					OctTreeNodeArray& node_array { std::get< OctTreeNodeArray >( m_node_data ) };
					//Search deeper

					node_array[ LEFT ][ FORWARD ][ TOP ]->getAllLeafsInFrustum( frustum, out_leafs );
					node_array[ LEFT ][ FORWARD ][ BOTTOM ]->getAllLeafsInFrustum( frustum, out_leafs );

					node_array[ LEFT ][ BACK ][ TOP ]->getAllLeafsInFrustum( frustum, out_leafs );
					node_array[ LEFT ][ BACK ][ BOTTOM ]->getAllLeafsInFrustum( frustum, out_leafs );

					node_array[ RIGHT ][ FORWARD ][ TOP ]->getAllLeafsInFrustum( frustum, out_leafs );
					node_array[ RIGHT ][ FORWARD ][ BOTTOM ]->getAllLeafsInFrustum( frustum, out_leafs );

					node_array[ RIGHT ][ BACK ][ TOP ]->getAllLeafsInFrustum( frustum, out_leafs );
					node_array[ RIGHT ][ BACK ][ BOTTOM ]->getAllLeafsInFrustum( frustum, out_leafs );
					return;
				}
			case 1: // NodeLeaf
				{
					assert( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) );
					OctTreeNodeLeaf& leaf { std::get< OctTreeNodeLeaf >( m_node_data ) };
					leafs.emplace_back( &leaf );

					//debug::world::drawBoundingBox( m_bounds );

					return;
				}
			default:
				FGL_UNREACHABLE();
		}

		FGL_UNREACHABLE();
	}

	bool OctTreeNode::contains( const WorldCoordinate coord ) const
	{
		return this->m_bounds.contains( coord );
	}

	OctTreeNode& OctTreeNode::operator[]( const WorldCoordinate coord )
	{
		assert( std::holds_alternative< OctTreeNodeArray >( m_node_data ) );
		const auto test_dim { glm::greaterThanEqual( coord.vec(), this->m_bounds.getPosition().vec() ) };

		auto& node_array { std::get< OctTreeNodeArray >( m_node_data ) };
		const auto& node { node_array[ test_dim.x ][ test_dim.y ][ test_dim.z ] };
		assert( node );

		return *node.get();
	}

	OctTreeNode::OctTreeNode( const WorldCoordinate center, float span, OctTreeNode* parent ) :
	  m_fit_bounding_box( center, glm::vec3( span, span, span ) ),
	  m_bounds( center, span ),
	  m_node_data( OctTreeNodeLeaf() ),
	  m_parent( parent )
	{
		assert( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) );
		std::get< OctTreeNodeLeaf >( m_node_data ).reserve( MAX_NODES_IN_LEAF );
	}

	void OctTreeNode::split( int depth )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeArray >( m_node_data ) ) return;
		auto& game_objects { std::get< OctTreeNodeLeaf >( m_node_data ) };

		//Figure out the half span
		const float half_span { m_bounds.span() / 2.0f };

		const Coordinate< CoordinateSpace::World > center { m_bounds.getPosition() };

		OctTreeNodeArray new_nodes {};

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

		[[assume( game_objects.size() <= MAX_NODES_IN_LEAF )]];

		for ( GameObject& obj : game_objects )
		{
			const auto& obj_coordinate { obj.getTransform().translation };
			const bool is_right { obj_coordinate.x > center.x };
			const bool is_forward { obj_coordinate.y > center.y };
			const bool is_up { obj_coordinate.z > center.z };

			std::unique_ptr< OctTreeNode >& node { new_nodes[ is_right ][ is_forward ][ is_up ] };
			assert( std::holds_alternative< OctTreeNodeLeaf >( node->m_node_data ) );

			std::get< OctTreeNodeLeaf >( node->m_node_data ).emplace_back( std::move( obj ) );
		}

		this->m_node_data = std::move( new_nodes );

		if ( depth - 1 >= 1 )
		{
			split( depth );
		}
	}

	OctTreeNode* OctTreeNode::addGameObject( GameObject&& obj )
	{
		assert( this->canContain( obj ) );
		if ( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) )
		{
			auto& objects { std::get< OctTreeNodeLeaf >( m_node_data ) };
			assert( objects.capacity() == MAX_NODES_IN_LEAF );
			// If the amount of nodes is about to exceed the number of leafs, Then split the nodes
			if ( objects.size() + 1 > MAX_NODES_IN_LEAF )
			{
				split();
				return this->addGameObject( std::move( obj ) );
			}

			log::debug( "Added game object" );
			objects.emplace_back( std::move( obj ) );
			return this;
		}

		return ( *this )[ obj.getPosition() ].addGameObject( std::forward< GameObject >( obj ) );
	}

	bool OctTreeNode::isInFrustum( const Frustum< CoordinateSpace::World >& frustum ) const
	{
#if ENABLE_IMGUI
		if ( !isEmpty() && frustum.intersects( m_fit_bounding_box ) )
		{
			if ( ( draw_inview_bounds || std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) ) && m_parent )
			{
				if ( draw_leaf_fit_bounds ) debug::world::drawBoundingBox( m_fit_bounding_box );
				if ( draw_leaf_real_bounds ) debug::world::drawBoundingBox( m_bounds );
			}

			return true;
		}
		else
			return false;

#else
		return !isEmpty() && frustum.intersects( m_fit_bounding_box );
#endif
	}

	OctTreeNode* OctTreeNode::findID( const GameObject::GameObjectID id )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) )
		{
			//We are the last node. Check if we have the ID
			const auto& game_objects { std::get< OctTreeNodeLeaf >( m_node_data ) };

			if ( std::find_if(
					 game_objects.begin(),
					 game_objects.end(),
					 [ id ]( const GameObject& obj ) { return obj.getId() == id; } )
			     != game_objects.end() )
			{
				return this;
			}
			else
			{
				return nullptr;
			}
		}
		else if ( std::holds_alternative< OctTreeNodeArray >( this->m_node_data ) )
		{
			const auto& node_array { std::get< OctTreeNodeArray >( this->m_node_data ) };

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

		FGL_UNREACHABLE();
	}

	auto OctTreeNode::getGameObjectItter( const GameObject::GameObjectID id )
	{
		assert( std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) );
		auto& game_objects { std::get< OctTreeNodeLeaf >( this->m_node_data ) };
		return std::find_if(
			game_objects.begin(), game_objects.end(), [ id ]( const GameObject& obj ) { return id == obj.getId(); } );
	}

	bool OctTreeNode::canContain( const GameObject& obj )
	{
		return m_bounds.contains( obj.getTransform().translation );
	}

	GameObject OctTreeNode::extract( const GameObject::GameObjectID id )
	{
		auto itter { getGameObjectItter( id ) };
		auto game_object { std::move( *itter ) };
		auto& game_objects { std::get< OctTreeNodeLeaf >( this->m_node_data ) };
		game_objects.erase( itter );
		return game_object;
	}

	OctTreeNode* OctTreeNode::getRoot()
	{
		if ( m_parent == nullptr ) return this;

		return m_parent->getRoot();
	}

	void OctTreeNode::getAllLeafs( std::vector< OctTreeNodeLeaf* >& objects )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) )
		{
			auto& leaf { std::get< OctTreeNodeLeaf >( m_node_data ) };
			//No point in us giving back an empty leaf
			if ( leaf.size() > 0 ) objects.emplace_back( &leaf );
		}
		else
		{
			auto& nodes { std::get< OctTreeNodeArray >( m_node_data ) };

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
	}

	/*
	bool OctTreeNode::recalculateBoundingBoxes()
	{
		ZoneScoped;
		const auto old_bounds { m_fit_bounding_box };
		if ( std::holds_alternative< OctTreeNodeArray >( m_node_data ) )
		{
			ZoneScopedN( "Process Array" );
			bool bounding_box_changed { false };
			auto& nodes { std::get< OctTreeNodeArray >( m_node_data ) };
			for ( std::size_t x = 0; x < 2; ++x )
			{
				for ( std::size_t y = 0; y < 2; ++y )
				{
					for ( std::size_t z = 0; z < 2; ++z )
					{
						auto& node { nodes[ x ][ y ][ z ] };
						bounding_box_changed |= node->recalculateBoundingBoxes();
					}
				}
			}

			if ( bounding_box_changed )
			{
				//We need to update our bounding box now.
				auto new_bounds { nodes[ 0 ][ 0 ][ 0 ]->m_fit_bounding_box };

				for ( std::size_t x = 0; x < 2; ++x )
				{
					for ( std::size_t y = 0; y < 2; ++y )
					{
						for ( std::size_t z = 0; z < 2; ++z )
						{
							if ( x == 0 && y == 0 && z == 0 ) continue;
							auto& node { nodes[ x ][ y ][ z ] };
							new_bounds.combine( node->m_fit_bounding_box );
						}
					}
				}

				if ( new_bounds == old_bounds )
				{
					return false;
				}
				else
				{
					this->m_fit_bounding_box = new_bounds;
					return true;
				}
			}

			return false;
		}
		else if ( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) )
		{
			ZoneScopedN( "Process Leaf" );
			auto& game_objects { std::get< OctTreeNodeLeaf >( m_node_data ) };

			if ( game_objects.size() == 0 ) return false;

			AxisAlignedBoundingBox< CoordinateSpace::World > new_bounds { game_objects[ 0 ].getPosition() };

			[[assume( game_objects.size() <= MAX_NODES_IN_LEAF )]];

			for ( std::size_t i = 1; i < game_objects.size(); ++i )
			{
				[[assume( i <= MAX_NODES_IN_LEAF )]];
				new_bounds.combine( game_objects[ i ].getBoundingBox() );
			}

			if ( new_bounds == old_bounds )
			{
				return false;
			}
			else
			{
				this->m_fit_bounding_box = new_bounds;
				return true;
			}
		}

		FGL_UNREACHABLE();
	}
	*/

	std::size_t OctTreeNode::reorganize()
	{
		std::size_t counter { 0 };
		if ( std::holds_alternative< LeafDataT >( m_node_data ) )
		{
			//Check if any of the nodes in this group need to be moved.

			for ( auto& game_objects = std::get< LeafDataT >( m_node_data ); const auto& game_object : game_objects )
			{
				if ( !this->canContain( game_object ) )
				{
					++counter;
					//Need to move this game object.
					auto moved_game_object { this->extract( game_object ) };

					//Insert at root
					//TODO: See if we can optimize this by traveling UP the tree.
					getRoot()->addGameObject( std::move( moved_game_object ) );
				}
			}
			return counter;
		}
		else if ( std::holds_alternative< NodeDataT >( m_node_data ) )
		{
			const auto& nodes { std::get< NodeDataT >( m_node_data ) };

			for ( std::size_t x = 0; x < 2; ++x )
			{
				for ( std::size_t y = 0; y < 2; ++y )
				{
					for ( std::size_t z = 0; z < 2; ++z )
					{
						if ( x == 0 && y == 0 && z == 0 ) continue;
						const auto& node { nodes[ x ][ y ][ z ] };
						counter += node->reorganize();
					}
				}
			}

			return counter;
		}

		FGL_UNREACHABLE();
	}

} // namespace fgl::engine