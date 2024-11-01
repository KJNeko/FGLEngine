//
// Created by kj16609 on 2/11/24.
//

#pragma once

#include <ostream>
#include <vector>

#include "engine/primitives/boxes/BoundingBox.hpp"
#include "engine/primitives/planes/PointPlane.hpp"
#include "engine/primitives/points/Coordinate.hpp"
#include "vectors/Vector.hpp"

namespace fgl::engine
{

	class Camera;

	//! Frustum constructed in model space (To be translated to a World space frustum later)
	struct FrustumBase
	{
		ModelPlane m_near { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		ModelPlane m_far { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		ModelPlane m_top { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		ModelPlane m_bottom { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		ModelPlane m_right { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		ModelPlane m_left { ModelCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };

		ModelCoordinate m_position {};

		FrustumBase() = default;

		FrustumBase(
			const ModelPlane& near_plane,
			const ModelPlane& far_plane,
			const ModelPlane& top_plane,
			const ModelPlane& bottom_plane,
			const ModelPlane& right_plane,
			const ModelPlane& left_plane,
			const ModelCoordinate position ) :
		  m_near( near_plane ),
		  m_far( far_plane ),
		  m_top( top_plane ),
		  m_bottom( bottom_plane ),
		  m_right( right_plane ),
		  m_left( left_plane ),
		  m_position( position )
		{}
	};

	struct Frustum
	{
		WorldPlane m_near { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		WorldPlane m_far { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		WorldPlane m_top { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		WorldPlane m_bottom { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		WorldPlane m_right { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };
		WorldPlane m_left { WorldCoordinate( constants::WORLD_CENTER ), NormalVector( constants::WORLD_Y ) };

		WorldCoordinate m_position {};

		friend class Camera;

	  public:

		Frustum() = default;

		//TODO: Change this to be, near far, top bottom, left right
		Frustum(
			const WorldPlane& near_plane,
			const WorldPlane& far_plane,
			const WorldPlane& top_plane,
			const WorldPlane& bottom_plane,
			const WorldPlane& right_plane,
			const WorldPlane& left_plane,
			const WorldCoordinate position ) :
		  m_near( near_plane ),
		  m_far( far_plane ),
		  m_top( top_plane ),
		  m_bottom( bottom_plane ),
		  m_right( right_plane ),
		  m_left( left_plane ),
		  m_position( position )
		{}

		FGL_FORCE_INLINE NormalVector forwardVec() const;
		FGL_FORCE_INLINE NormalVector upVec() const;
		FGL_FORCE_INLINE NormalVector rightVec() const;

		WorldCoordinate getPosition() const { return m_position; }

		//! Tests if a point is inside of the frustum
		bool containsPoint( const WorldCoordinate coord ) const;
		//! Returns true if ANY point is inside the frustum
		bool containsAnyPoint( const std::vector< WorldCoordinate >& coords ) const;
		//! Used for bounding box tests
		bool containsAnyPoint( const std::array< WorldCoordinate, interface::BoundingBox::POINT_COUNT >& coords ) const;

		std::array< WorldCoordinate, 4 * 2 > points() const;
		std::array< LineSegment< CoordinateSpace::World >, ( ( 4 * 2 ) / 2 ) * 3 > lines() const;
	};

#ifdef EXPOSE_FRUSTUM_INTERNALS
	float signedDistance( const Vector direction, const WorldCoordinate& point, const WorldCoordinate& origin );
	void processPlane(
		const WorldPlane plane,
		const Line< CoordinateSpace::World > line,
		std::vector< WorldCoordinate >& out_enter_intersections,
		std::vector< WorldCoordinate >& out_exit_intersections );

	WorldCoordinate getFirstExit(
		const std::vector< WorldCoordinate >& enter_intersections, const Line< CoordinateSpace::World > line );
	WorldCoordinate getLastEnter(
		const std::vector< WorldCoordinate >& exit_intersections, const Line< CoordinateSpace::World > line );
#endif

	void imGuiFrustumSettings();

	Frustum operator*( const Matrix< MatrixType::ModelToWorld >& matrix, const FrustumBase& frustum );

} // namespace fgl::engine
