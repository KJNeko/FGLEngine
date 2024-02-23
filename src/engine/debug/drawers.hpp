//
// Created by kj16609 on 1/28/24.
//

#pragma once

#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Frustum.hpp"
#include "engine/primitives/Line.hpp"
#include "engine/primitives/Plane.hpp"

namespace fgl::engine
{
	template < CoordinateSpace type >
	class BoundingBox;

	template < CoordinateSpace type >
	class Coordinate;

	template < CoordinateSpace type >
	class Line;

	class Camera;

	struct SinglePointDistancePlane;

} // namespace fgl::engine

#ifndef NDEBUG
namespace fgl::engine::debug
{

	namespace world
	{
		void drawBoundingBox(
			const BoundingBox< CoordinateSpace::World >& box,
			Camera& camera,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawLine(
			const Coordinate< CoordinateSpace::World > start,
			const Coordinate< CoordinateSpace::World > end,
			const Camera& camera,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawPointLabel(
			const Coordinate< CoordinateSpace::World > point, const std::string label, const Camera& camera );

		void drawLine(
			const Line< CoordinateSpace::World > line,
			const Camera& camera,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawPointText(
			const Coordinate< CoordinateSpace::World > point,
			const Camera& camera,
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawBoolAlpha(
			const Coordinate< CoordinateSpace::World > point,
			const Camera& camera,
			const bool value,
			const glm::vec2 offset = {} );
		void drawPoint(
			const Coordinate< CoordinateSpace::World > point,
			const Camera& camera,
			const std::string label = "",
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawVector(
			const Coordinate< CoordinateSpace::World > point,
			Vector vector,
			const Camera& camera,
			const std::string label = "",
			const glm::vec3 color = { 1.0f, 1.0f, 1.0f } );

		void drawFrustum(
			const Frustum< CoordinateSpace::World >& frustum, const Camera& camera, const WorldCoordinate coordinate );
		void drawFrustum( const Camera& camera );

		void drawPlane(
			const Plane< CoordinateSpace::World >& plane,
			const WorldCoordinate point,
			const Camera& camera,
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
	namespace world
	{
		//Dummy functions
		inline void drawBoundingBox( const BoundingBox< CoordinateSpace::World >&, Camera&, const glm::vec3 )
		{}

		inline void drawBoundingBox( const BoundingBox< CoordinateSpace::World >&, Camera& )
		{}

		inline void drawLine(
			const Coordinate< CoordinateSpace::World >,
			const Coordinate< CoordinateSpace::World >,
			const Camera&,
			const glm::vec3 )
		{}

		inline void drawPointLabel( const Coordinate< CoordinateSpace::World >, const std::string, const Camera& )
		{}

		inline void drawLine( const Line< CoordinateSpace::World >, const Camera&, const glm::vec3 )
		{}

		inline void drawPointText( const Coordinate< CoordinateSpace::World >, const Camera&, const glm::vec3 )
		{}

		inline void
			drawBoolAlpha( const Coordinate< CoordinateSpace::World >, const Camera&, const bool, const glm::vec2 )
		{}

		inline void
			drawPoint( const Coordinate< CoordinateSpace::World >, const Camera&, const std::string, const glm::vec3 )
		{}

		inline void drawVector(
			const Coordinate< CoordinateSpace::World >, Vector, const Camera&, const std::string, const glm::vec3 )
		{}

		inline void drawFrustum( const Frustum< CoordinateSpace::World >&, const Camera&, const WorldCoordinate )
		{}

		inline void drawFrustum( const Camera& )
		{}

		inline void drawPlane(
			const Plane< CoordinateSpace::World >&,
			const WorldCoordinate,
			const Camera&,
			const std::string,
			const glm::vec3 )
		{}

	} // namespace world

} // namespace fgl::engine::debug

#endif
