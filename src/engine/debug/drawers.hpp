//
// Created by kj16609 on 1/28/24.
//

#pragma once

#include <memory>

#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/lines/LineSegment.hpp"
#include "engine/primitives/planes/PointPlane.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	template < CoordinateSpace type >
	struct OrientedBoundingBox;
	template < CoordinateSpace type >
	class AxisAlignedBoundingBox;
	template < CoordinateSpace type >
	class AxisAlignedBoundingCube;

	template < CoordinateSpace type >
	class Coordinate;

	template < CoordinateSpace type >
	class LineSegment;

	class Camera;

	struct SinglePointDistancePlane;

} // namespace fgl::engine

#ifndef ENABLE_IMGUI_DRAWERS
#define ENABLE_IMGUI_DRAWERS 0
#endif

#ifndef ENABLE_IMGUI
#define ENABLE_IMGUI 0
#endif

#if ENABLE_IMGUI_DRAWERS
namespace fgl::engine::debug
{
	Camera& getDebugDrawingCamera();
	void setDebugDrawingCamera( std::shared_ptr< Camera >& );

	namespace world
	{
		void drawBoundingBox(
			const OrientedBoundingBox< CoordinateSpace::World >& box, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );
		void drawBoundingBox(
			const AxisAlignedBoundingBox< CoordinateSpace::World >& box, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );
		void drawBoundingBox(
			const AxisAlignedBoundingCube< CoordinateSpace::World >& box, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawLine(
			Coordinate< CoordinateSpace::World > start,
			Coordinate< CoordinateSpace::World > end,
			glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawPointLabel( Coordinate< CoordinateSpace::World > point, std::string label );

		void drawLineI(
			LineSegment< CoordinateSpace::World > line,
			glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			float thickness = 1.0f );
		void drawLine(
			LineSegment< CoordinateSpace::World > line,
			glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			float thickness = 1.0f );

		void drawPointText( Coordinate< CoordinateSpace::World > point, glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawBoolAlpha( Coordinate< CoordinateSpace::World > point, bool value, glm::vec2 offset = {} );
		void drawPoint(
			Coordinate< CoordinateSpace::World > point,
			std::string label = "",
			glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawVector(
			Coordinate< CoordinateSpace::World > point,
			Vector vector,
			std::string label = "",
			glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawVector(
			Coordinate< CoordinateSpace::World > point,
			NormalVector vector,
			std::string label = "",
			glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawFrustum( const Frustum< CoordinateSpace::World >& frustum, WorldCoordinate coordinate );
		void drawFrustum();

		void drawPlane(
			const Plane< CoordinateSpace::World >& plane,
			WorldCoordinate point,
			std::string label = "",
			glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

	} // namespace world

	namespace screen
	{

		void drawText(
			glm::vec2 position,
			const std::string& text,
			glm::vec3 color = { 1.0f, 1.0f, 1.0f },
			glm::vec2 offset = {} );

		void drawBoolAlpha( glm::vec2 screen_point, const Camera& camera, bool value, glm::vec2 offset = {} );

	} // namespace screen

} // namespace fgl::engine::debug
#else

namespace fgl::engine::debug
{
	inline void setDebugDrawingCamera( [[maybe_unused]] Camera& cam )
	{}

	namespace world
	{

		//Dummy functions
		inline void drawBoundingBox(
			[[maybe_unused]] const OrientedBoundingBox< CoordinateSpace::World >&, [[maybe_unused]] const glm::vec3 )
		{}

		inline void drawBoundingBox( [[maybe_unused]] const OrientedBoundingBox< CoordinateSpace::World >& )
		{}

		inline void drawLine(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] const glm::vec3,
			[[maybe_unused]] const float )
		{}

		inline void drawPointLabel(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >, [[maybe_unused]] const std::string )
		{}

		inline void drawLine(
			[[maybe_unused]] const Line< CoordinateSpace::World >,
			[[maybe_unused]] const glm::vec3,
			[[maybe_unused]] const float )
		{}

		inline void drawPointText(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >, [[maybe_unused]] const glm::vec3 )
		{}

		inline void drawBoolAlpha(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] const Camera&,
			[[maybe_unused]] const bool,
			[[maybe_unused]] const glm::vec2 )
		{}

		inline void drawPoint(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] const std::string,
			[[maybe_unused]] const glm::vec3 )
		{}

		inline void drawVector(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] Vector,
			[[maybe_unused]] const std::string,
			[[maybe_unused]] const glm::vec3 )
		{}

		inline void drawVector(
			[[maybe_unused]] const Coordinate< CoordinateSpace::World >,
			[[maybe_unused]] NormalVector,
			[[maybe_unused]] const std::string,
			[[maybe_unused]] const glm::vec3 )
		{}

		inline void drawFrustum(
			[[maybe_unused]] const Frustum< CoordinateSpace::World >&, [[maybe_unused]] const WorldCoordinate )
		{}

		inline void drawFrustum()
		{}

		inline void drawPlane(
			[[maybe_unused]] const Plane< CoordinateSpace::World >&,
			[[maybe_unused]] const WorldCoordinate,
			[[maybe_unused]] const std::string,
			[[maybe_unused]] const glm::vec3 )
		{}

	} // namespace world

} // namespace fgl::engine::debug

#endif
