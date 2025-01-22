//
// Created by kj16609 on 3/1/24.
//

#include "OctTreeNode.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <engine/FrameInfo.hpp>
#include <glm/gtx/string_cast.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#include <imgui.h>
#pragma GCC diagnostic pop

#include "engine/assets/model/Model.hpp"
#include "engine/clock.hpp"
#include "engine/debug/drawers.hpp"
#include "engine/math/intersections.hpp"
#include "engine/primitives/Frustum.hpp"

namespace fgl::engine
{
	struct FrameInfo;

	static bool draw_leaf_fit_bounds { false };
	static bool draw_leaf_real_bounds { false };
	static bool draw_inview_bounds { false };
	static bool draw_branches { false };
	static bool draw_model_bounding_boxes { false };
	static std::size_t number_moved { 0 };
	static std::optional< std::chrono::microseconds > time { std::nullopt };

	void imGuiOctTreeSettings( const FrameInfo& info )
	{
#if ENABLE_IMGUI
		if ( ImGui::CollapsingHeader( "OctTree debug settings" ) )
		{
			ImGui::Checkbox( "Draw leaf fitted bounding boxes", &draw_leaf_fit_bounds );
			ImGui::Checkbox( "Draw leaf real bounding boxes", &draw_leaf_real_bounds );
			ImGui::Checkbox( "Draw ALL in view bounding boxes", &draw_inview_bounds );
			ImGui::Checkbox( "Draw branches", &draw_branches );
			ImGui::Checkbox( "Draw all model bounding boxes", &draw_model_bounding_boxes );

			if ( ImGui::Button( "Reorganize Octtree" ) )
			{
				const auto start { fgl::Clock::now() };
				number_moved = info.game_objects.reorganize();
				const auto end { fgl::Clock::now() };
				const auto time_diff { end - start };
				time = std::chrono::duration_cast< std::chrono::microseconds >( time_diff );
			}

			if ( ImGui::Button( "Recalculate Bounds" ) )
			{
				const auto start { fgl::Clock::now() };
				info.game_objects.recalculateChildBounds();
				const auto end { fgl::Clock::now() };
				const auto time_diff { end - start };
				time = std::chrono::duration_cast< std::chrono::microseconds >( time_diff );
			}

			if ( time.has_value() )
			{
				ImGui::Text( "Time spent reorganizing: %.2ldus", time.value().count() );
				ImGui::Text( "Moved %ld objects", number_moved );
			}
		}
#endif
	}

	void OctTreeNode::getAllLeafsInFrustum( const Frustum& frustum, std::vector< OctTreeNodeLeaf* >& out_leafs )
	{
		ZoneScoped;

		auto& leafs { out_leafs };

		switch ( m_node_data.index() )
		{
			case 0: // NodeArray
				{
					//Check if we are inside the frustum.
					if ( !isInFrustum( frustum ) ) return;

					assert( std::holds_alternative< OctTreeNodeArray >( m_node_data ) );
					const OctTreeNodeArray& node_array { std::get< OctTreeNodeArray >( m_node_data ) };
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
					OctTreeNodeLeaf& leaf { std::get< OctTreeNodeLeaf >( m_node_data ) };
					if ( leaf.empty() ) return;

					//Check if we are inside the frustum.
					if ( !isInFrustum( frustum ) ) return;

					assert( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) );
					leafs.emplace_back( &leaf );

					//debug::world::drawBoundingBox( m_bounds );

					return;
				}
			default:
				FGL_UNREACHABLE();
		}

		FGL_UNREACHABLE();
	}

	OctTreeNode& OctTreeNode::operator[]( const WorldCoordinate coord ) const
	{
		assert( std::holds_alternative< OctTreeNodeArray >( m_node_data ) );

		// Bounding box center
		const auto bounds_center { this->m_bounds.getPosition().vec() };

		const auto coordinate_center { coord.vec() };

		//const auto test_dim { glm::greaterThanEqual( coord.vec(), bounds_center ) };
		//const auto& node_array { std::get< OctTreeNodeArray >( m_node_data ) };
		//const auto& node { node_array[ test_dim.x ? 1 : 0 ][ test_dim.y ? 1 : 0 ][ test_dim.z ? 1 : 0 ] };

		const std::size_t x_idx { coordinate_center.x > bounds_center.x ? 1ul : 0ul };
		const std::size_t y_idx { coordinate_center.y > bounds_center.y ? 1ul : 0ul };
		const std::size_t z_idx { coordinate_center.z > bounds_center.z ? 1ul : 0ul };

		const auto& node { std::get< OctTreeNodeArray >( m_node_data )[ x_idx ][ y_idx ][ z_idx ] };

		FGL_ASSERT( node, "Node was invalid!" );
		FGL_ASSERT( node->canContain( coord ), "Node was not capable of containing the object!" );

		return *node.get();
	}

	OctTreeNode::OctTreeNode( const WorldCoordinate center, const float span, OctTreeNode* parent ) :
	  m_fit_bounding_box( center, glm::vec3( glm::abs( span ) ) ),
	  m_bounds( center, glm::abs( span ) ),
	  m_node_data( OctTreeNodeLeaf() ),
	  m_parent( parent )
	{
		assert( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) );
		std::get< OctTreeNodeLeaf >( m_node_data ).reserve( MAX_NODES_IN_LEAF );
	}

	OctTreeNode::~OctTreeNode()
	{}

	void OctTreeNode::split( const int depth )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeArray >( m_node_data ) ) return;
		auto game_objects { std::get< OctTreeNodeLeaf >( std::move( m_node_data ) ) };

		//Figure out the half span
		const float half_span { m_bounds.span() / 2.0f };

		const Coordinate< CoordinateSpace::World > center { m_bounds.getPosition() };

		OctTreeNodeArray new_nodes {};

		const float left_x { center.x - half_span };
		const float right_x { center.x + half_span };

		const float forward_y { center.y + half_span };
		const float backward_y { center.y - half_span };

		const float top_z { center.z + half_span };
		const float bottom_z { center.z - half_span };

		assert( !std::isinf( left_x ) && !std::isinf( right_x ) );
		assert( !std::isinf( forward_y ) && !std::isinf( backward_y ) );
		assert( !std::isinf( top_z ) && !std::isinf( bottom_z ) );

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

		FGL_ASSUME( game_objects.size() <= MAX_NODES_IN_LEAF )

		for ( GameObject& obj : game_objects )
		{
			const auto& obj_coordinate { obj.getTransform().translation };
			const bool is_right { obj_coordinate.x > center.x };
			const bool is_forward { obj_coordinate.y > center.y };
			const bool is_up { obj_coordinate.z > center.z };

			const std::unique_ptr< OctTreeNode >& node {
				new_nodes[ is_right ? 1 : 0 ][ is_forward ? 1 : 0 ][ is_up ? 1 : 0 ]
			};
			assert( std::holds_alternative< OctTreeNodeLeaf >( node->m_node_data ) );

			std::get< OctTreeNodeLeaf >( node->m_node_data ).emplace_back( std::move( obj ) );
		}

		this->m_node_data = std::move( new_nodes );

		recalculateChildBounds();

		if ( depth - 1 >= 1 )
		{
			split( depth );
		}
	}

	OctTreeNode* OctTreeNode::addGameObject( GameObject&& obj )
	{
		assert( this->canContain( obj ) );

		if ( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) ) // This node is a leaf
		{
			auto& objects { std::get< OctTreeNodeLeaf >( m_node_data ) };
			assert( objects.capacity() == MAX_NODES_IN_LEAF );
			// If the amount of nodes is about to exceed the number of leafs, Then split the nodes
			if ( objects.size() + 1 > MAX_NODES_IN_LEAF )
			{
				split();
				auto* node { this->addGameObject( std::move( obj ) ) };
				return node;
			}

			const bool should_recalc_bounds { obj.hasComponent< ModelComponent >() };

			objects.emplace_back( std::move( obj ) );

			if ( should_recalc_bounds ) recalculateLeafBounds();

			return this;
		}

		if ( std::holds_alternative< OctTreeNodeArray >( m_node_data ) )
		{
			auto* node { ( *this )[ obj.getPosition() ].addGameObject( std::forward< GameObject >( obj ) ) };
			return node;
		}

		FGL_UNREACHABLE();
	}

	bool OctTreeNode::isLeaf() const
	{
		return std::holds_alternative< OctTreeNodeLeaf >( m_node_data );
	}

	bool OctTreeNode::isBranch() const
	{
		return std::holds_alternative< OctTreeNodeArray >( m_node_data );
	}

	std::size_t OctTreeNode::itemCount() const
	{
		//TODO: Store this value in the nodes itself
		if ( !isLeaf() )
		{
			std::size_t sum { 0 };

			FOR_EACH_OCTTREE_NODE
			{
				sum += std::get< OctTreeNodeArray >( m_node_data )[ x ][ y ][ z ]->itemCount();
			}

			return sum;
		}

		return std::get< OctTreeNodeLeaf >( m_node_data ).size();
	}

	const OctTreeNodeArray& OctTreeNode::getBranches() const
	{
		return std::get< OctTreeNodeArray >( m_node_data );
	}

	const OctTreeNodeLeaf& OctTreeNode::getLeaf() const
	{
		return std::get< OctTreeNodeLeaf >( m_node_data );
	}

	OctTreeNodeLeaf& OctTreeNode::getLeaf()
	{
		return std::get< OctTreeNodeLeaf >( m_node_data );
	}

	bool OctTreeNode::isInFrustum( const Frustum& frustum ) const
	{
#if ENABLE_IMGUI
		if ( !isEmpty() && intersects( frustum, m_fit_bounding_box ) )
		{
			if ( isLeaf() && itemCount() > 0 ) [[unlikely]]
			{
				if ( draw_leaf_fit_bounds ) [[unlikely]]
					debug::drawBoundingBox( m_fit_bounding_box );
				if ( draw_leaf_real_bounds ) [[unlikely]]
					debug::drawBoundingBox( m_bounds );
				if ( draw_model_bounding_boxes ) [[unlikely]]
				{
					for ( const auto& obj : getLeaf() )
					{
						const Matrix< MatrixType::ModelToWorld > obj_transform { obj.getTransform().mat() };

						for ( const auto* model : obj.getComponents< ModelComponent >() )
						{
							const auto model_bounds { ( *model )->getBoundingBox() };
							const Matrix< MatrixType::ModelToWorld > model_transform { model->m_transform.mat() };

							const auto combined_transform { obj_transform * model_transform };

							debug::drawBoundingBox( combined_transform * model_transform );
						}
					}
				}
			}

			return true;
		}

		return false;

#else
		return !isEmpty() && intersects( frustum, m_fit_bounding_box );
#endif
	}

	bool OctTreeNode::isEmpty() const
	{
		return std::holds_alternative< OctTreeNodeLeaf >( m_node_data )
		    && std::get< OctTreeNodeLeaf >( m_node_data ).empty();
	}

	/**
	 *
	 * @return Returns true if the fit bounding box is larger than the virtual bounds
	 */
	void OctTreeNode::recalculateBounds()
	{
		if ( isBranch() ) [[likely]]
		{
			recalculateNodeBounds();
			return;
		}
		else
		{
			FGL_ASSERT( isLeaf(), "Expected leaf, Got whatever the fuck this is instead" );
			recalculateLeafBounds();
			return;
		}

		FGL_UNREACHABLE();
	}

	std::vector< OctTreeNodeLeaf* > OctTreeNode::getAllLeafs()
	{
		ZoneScoped;
		std::vector< OctTreeNodeLeaf* > leafs {};
		leafs.reserve( LEAF_RESERVE_SIZE );
		this->getAllLeafs( leafs );
		return leafs;
	}

	std::vector< OctTreeNodeLeaf* > OctTreeNode::getAllLeafsInFrustum( const Frustum& frustum )
	{
		ZoneScoped;
		std::vector< OctTreeNodeLeaf* > leafs {};
		leafs.reserve( LEAF_RESERVE_SIZE );
		this->getAllLeafsInFrustum( frustum, leafs );
		return leafs;
	}

	void OctTreeNode::clear()
	{
		if ( std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) )
		{
			std::get< OctTreeNodeLeaf >( this->m_node_data ).clear();
		}
		else if ( std::holds_alternative< OctTreeNodeArray >( this->m_node_data ) )
		{
			const auto& node_array { std::get< OctTreeNodeArray >( this->m_node_data ) };

			FOR_EACH_OCTTREE_NODE
			{
				node_array[ x ][ y ][ z ]->clear();
			}
		}
	}

	WorldCoordinate OctTreeNode::getCenter() const
	{
		return m_bounds.getPosition();
	}

	WorldCoordinate OctTreeNode::getFitCenter() const
	{
		return m_fit_bounding_box.getPosition();
	}

	void OctTreeNode::drawDebug() const
	{}

	void OctTreeNode::recalculateChildBounds()
	{
		if ( isBranch() )
		{
			FOR_EACH_OCTTREE_NODE
			{
				auto& node { std::get< OctTreeNodeArray >( m_node_data )[ x ][ y ][ z ] };
				node->recalculateChildBounds();
			}
		}
		else
		{
			recalculateBounds();
		}
	}

	OctTreeNode* OctTreeNode::findID( const GameObject::GameObjectID id )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) )
		{
			//We are the last node. Check if we have the ID
			const auto& game_objects { std::get< OctTreeNodeLeaf >( m_node_data ) };

			if ( std::ranges::find_if( game_objects, [ id ]( const GameObject& obj ) { return obj.getId() == id; } )
			     != game_objects.end() )
			{
				return this;
			}

			return nullptr;
		}

		if ( std::holds_alternative< OctTreeNodeArray >( this->m_node_data ) )
		{
			const auto& node_array { std::get< OctTreeNodeArray >( this->m_node_data ) };

			FOR_EACH_OCTTREE_NODE
			{
				const auto& node { node_array[ x ][ y ][ z ]->findID( id ) };
				if ( node != nullptr ) return node;
			}

			return nullptr;
		}

		FGL_UNREACHABLE();
	}

	auto OctTreeNode::getGameObjectItter( const GameObject::GameObjectID id )
	{
		assert( std::holds_alternative< OctTreeNodeLeaf >( this->m_node_data ) );
		auto& game_objects { std::get< OctTreeNodeLeaf >( this->m_node_data ) };
		return std::ranges::
			find_if( game_objects, [ id ]( const GameObject& obj ) noexcept { return id == obj.getId(); } );
	}

	bool OctTreeNode::canContain( const GameObject& obj ) const
	{
		return canContain( obj.getTransform().translation );
	}

	bool OctTreeNode::canContain( const WorldCoordinate& coord ) const
	{
		const auto center { this->getCenter() };
		// top right forward
		const auto high_center { center.vec() + glm::vec3( this->m_bounds.scale() ) };
		// bottom left back
		const auto low_center { center.vec() - glm::vec3( this->m_bounds.scale() ) };

		const bool under_high_center { glm::all( glm::lessThanEqual( coord.vec(), high_center ) ) };
		const bool above_low_center { glm::all( glm::greaterThan( coord.vec(), low_center ) ) };

		return under_high_center && above_low_center;
	}

	GameObject OctTreeNode::extract( const GameObject::GameObjectID id )
	{
		const auto itter { getGameObjectItter( id ) };
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

	void OctTreeNode::getAllLeafs( std::vector< OctTreeNodeLeaf* >& out_leafs )
	{
		ZoneScoped;
		if ( std::holds_alternative< OctTreeNodeLeaf >( m_node_data ) )
		{
			auto& leaf { std::get< OctTreeNodeLeaf >( m_node_data ) };
			//No point in us giving back an empty leaf
			if ( !leaf.empty() ) out_leafs.emplace_back( &leaf );
		}
		else
		{
			const auto& nodes { std::get< OctTreeNodeArray >( m_node_data ) };

			FOR_EACH_OCTTREE_NODE
			{
				auto ret { nodes[ x ][ y ][ z ]->getAllLeafs() };
				out_leafs.insert( out_leafs.end(), ret.begin(), ret.end() );
			}
		}
	}

	std::size_t OctTreeNode::reorganize()
	{
		std::size_t counter { 0 };
		if ( std::holds_alternative< NodeDataT >( m_node_data ) )
		{
			const auto& nodes { std::get< NodeDataT >( m_node_data ) };

			FOR_EACH_OCTTREE_NODE
			{
				// Why did I skip the 0,0,0 node previously?
				// if ( x == 0 && y == 0 && z == 0 ) continue;
				const auto& node { nodes[ x ][ y ][ z ] };
				counter += node->reorganize();
			}

			return counter;
		}

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

		FGL_UNREACHABLE();
	}

	bool OctTreeNode::isBoundsExpanded() const
	{
		return m_fit_bounding_box == m_bounds;
		/*
		const auto fit_points { m_fit_bounding_box.points() };
		for ( const auto& p : fit_points )
		{
			// Return true if a point is outside the bounds. This indicates that out bounding box is bigger than our bounds.
			if ( !m_bounds.contains( p ) ) return true;
		}

		return false;
		*/
	}

	void OctTreeNode::recalculateNodeBounds()
	{
		FGL_ASSERT( std::holds_alternative< NodeDataT >( m_node_data ), "Node data was not an array!" );
		const auto& nodes { std::get< NodeDataT >( m_node_data ) };

		// We start out by telling all of our children to recalculate their bounds
		m_fit_bounding_box = static_cast< AxisAlignedBoundingBox< CoordinateSpace::World > >( m_bounds );

		FOR_EACH_OCTTREE_NODE
		{
			// If true then the bounds were bigger then the inital bounding box. So we should try to combine it without current bounding box.
			m_fit_bounding_box = m_fit_bounding_box.combine( nodes[ x ][ y ][ z ]->m_fit_bounding_box );
		}

		// if ( isBoundsExpanded() && m_parent )
		if ( m_parent != nullptr ) m_parent->recalculateBounds();
	}

	void OctTreeNode::recalculateLeafBounds()
	{
		FGL_ASSERT( std::holds_alternative< LeafDataT >( m_node_data ), "Node data was not a leaf!" );
		const auto& data { std::get< LeafDataT >( m_node_data ) };

		m_fit_bounding_box = static_cast< AxisAlignedBoundingBox< CoordinateSpace::World > >( m_bounds );

		if ( data.empty() ) return;

		// If true, Then the fit has already been set, and we should combine with it
		bool fit_set { false };

		for ( const auto& game_object : data )
		{
			const Matrix< MatrixType::ModelToWorld > game_object_transform { game_object.getTransform().mat() };

			for ( const ModelComponent* model : game_object.getComponents< ModelComponent >() )
			{
				const OrientedBoundingBox< CS::Model > model_bounding_box { ( *model )->getBoundingBox() };

				const Matrix< MatrixType::ModelToWorld > model_transform { model->m_transform.mat() };

				// Combine the game object and model transform
				const Matrix< MatrixType::ModelToWorld > combined_transform { model_transform * game_object_transform };

				const OrientedBoundingBox< CoordinateSpace::World > world_bounding_box { combined_transform
					                                                                     * model_bounding_box };

				const auto aligned_bounding_box { world_bounding_box.alignToWorld() };

				if ( fit_set ) [[likely]]
					m_fit_bounding_box = m_fit_bounding_box.combine( aligned_bounding_box );
				else
				{
					m_fit_bounding_box = aligned_bounding_box;
					fit_set = true;
				}
			}
		}

		// Have our parent recalculate its bounds
		// if ( isBoundsExpanded() && m_parent )
		if ( m_parent != nullptr ) m_parent->recalculateBounds();
	}

} // namespace fgl::engine