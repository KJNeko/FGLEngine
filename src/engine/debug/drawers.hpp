//
// Created by kj16609 on 1/28/24.
//

#pragma once

#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Line.hpp"
#include "engine/primitives/PointPlane.hpp"

namespace fgl::engine
{
	template < CoordinateSpace type >
	struct OrientedBoundingBox;

	template < CoordinateSpace type >
	class Coordinate;

	template < CoordinateSpace type >
	struct Line;

	class Camera;

	struct SinglePointDistancePlane;

} // namespace fgl::engine

#ifndef ENABLE_IMGUI_DRAWERS
#define ENABLE_IMGUI_DRAWERS 0
#endif

#if ENABLE_IMGUI_DRAWERS
namespace fgl::engine::debug
{
	Camera& getDebugDrawingCamera();
	void setDebugDrawingCamera( Camera& );

	namespace world
	{
		void drawBoundingBox(
			const OrientedBoundingBox< CoordinateSpace::World >& box, const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawLine(
			const Coordinate< CoordinateSpace::World > start,
			const Coordinate< CoordinateSpace::World > end,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawPointLabel( const Coordinate< CoordinateSpace::World > point, const std::string label );

		void drawLineI(
			const Line< CoordinateSpace::World > line,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			const float thickness = 1.0f );
		void drawLine(
			const Line< CoordinateSpace::World > line,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			const float thickness = 1.0f );

		void drawPointText(
			const Coordinate< CoordinateSpace::World > point, const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawBoolAlpha(
			const Coordinate< CoordinateSpace::World > point, const bool value, const glm::vec2 offset = {} );
		void drawPoint(
			const Coordinate< CoordinateSpace::World > point,
			const std::string label = "",
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawVector(
			const Coordinate< CoordinateSpace::World > point,
			Vector vector,
			const std::string label = "",
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawFrustum( const Frustum< CoordinateSpace::World >& frustum, const WorldCoordinate coordinate );
		void drawFrustum();

		void drawPlane(
			const Plane< CoordinateSpace::World >& plane,
			const WorldCoordinate point,
			const std::string label = "",
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

	} // namespace world

	namespace screen
	{

		void drawText(
			const glm::vec2 position,
			const std::string& text,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			const glm::vec2 offset = {} );

		void drawBoolAlpha(
			const glm::vec2 screen_point, const Camera& camera, const bool value, const glm::vec2 offset = {} );

	} // namespace screen

} // namespace fgl::engine::debug
#else

namespace fgl::engine::debug
{
	inline void setDebugDrawingCamera( Camera& cam )
	{}

	namespace world
	{

		//Dummy functions
		inline void drawBoundingBox( const BoundingBox< CoordinateSpace::World >&, const glm::vec3 )
		{}

		inline void drawBoundingBox( const BoundingBox< CoordinateSpace::World >& )
		{}

		inline void drawLine(
			const Coordinate< CoordinateSpace::World >,
			const Coordinate< CoordinateSpace::World >,
			const glm::vec3,
			const float )
		{}

		inline void drawPointLabel( const Coordinate< CoordinateSpace::World >, const std::string )
		{}

		inline void drawLine( const Line< CoordinateSpace::World >, const glm::vec3, const float )
		{}

		inline void drawPointText( const Coordinate< CoordinateSpace::World >, const glm::vec3 )
		{}

		inline void
			drawBoolAlpha( const Coordinate< CoordinateSpace::World >, const Camera&, const bool, const glm::vec2 )
		{}

		inline void drawPoint( const Coordinate< CoordinateSpace::World >, const std::string, const glm::vec3 )
		{}

		inline void drawVector( const Coordinate< CoordinateSpace::World >, Vector, const std::string, const glm::vec3 )
		{}

		inline void drawFrustum( const Frustum< CoordinateSpace::World >&, const WorldCoordinate )
		{}

		inline void drawFrustum()
		{}

		inline void drawPlane(
			const Plane< CoordinateSpace::World >&,
			const WorldCoordinate,

			const std::string,
			const glm::vec3 )
		{}

	} // namespace world

} // namespace fgl::engine::debug

#endif
