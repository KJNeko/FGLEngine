//
// Created by kj16609 on 1/28/24.
//

#include "drawers.hpp"

#include "engine/Camera.hpp"
#include "engine/model/OrientedBoundingBox.hpp"
#include "imgui/imgui.h"
#include "tracy_colors.hpp"

#if ENABLE_IMGUI_DRAWERS

namespace fgl::engine::debug
{
	inline static std::optional< Camera* > debug_camera { std::nullopt };

	Camera& getDebugDrawingCamera()
	{
		assert( debug_camera.has_value() && "Debug camera not set" );
		return *debug_camera.value();
	}

	void setDebugDrawingCamera( Camera& cam )
	{
		debug_camera = &cam;
	}

	const ImVec2 windowSize()
	{
		return ImGui::GetMainViewport()->Size;
	}

	Coordinate< CoordinateSpace::Screen > toScreenSpace( Coordinate< CoordinateSpace::World > world_point )
	{
		const ImVec2 window_size { windowSize() };

		const Coordinate< CoordinateSpace::Screen > screen_point { glm::projectZO(
			static_cast< glm::vec3 >( world_point ),
			glm::mat4( 1.0f ),
			getDebugDrawingCamera().getProjectionViewMatrix(),
			glm::vec4( 0.0f, 0.0f, window_size.x, window_size.y ) ) };

		return screen_point;
	}

	ImVec2 glmToImgui( const glm::vec2 vec )
	{
		return ImVec2( vec.x, vec.y );
	}

	ImVec2 glmToImgui( const Coordinate< CoordinateSpace::Screen > coordinate )
	{
		return glmToImgui( static_cast< glm::vec3 >( coordinate ) );
	}

	bool isBehind( const glm::vec3 point )
	{
		return point.z > 1.0f || point.z < 0.0f;
	}

	bool inView( const glm::vec3 point )
	{
		const ImVec2 window_size { windowSize() };

		return !isBehind( point ) && ( point.x > 0.0f && point.x < window_size.x )
		    && ( point.y > 0.0f && point.y < window_size.y );
	}

	namespace world
	{

		inline void drawLineI( const Line< CoordinateSpace::World > line, const glm::vec3 color, const float thickness )
		{
			//Check if the line in intersecting the frustum
			if ( getDebugDrawingCamera().getFrustumBounds().intersects( line ) )
				drawLine( line, glm::vec3( 0.0f, 1.0f, 0.0f ), thickness );
			else
				drawLine( line, color, thickness );
		}

		void drawBoundingBox( const OrientedBoundingBox< CoordinateSpace::World >& box, const glm::vec3 color )
		{
			ZoneScopedC( TRACY_DRAWER_FUNC_COLOR );
			for ( const auto [ p1, p2 ] : box.lines() )
			{
				drawLine( { p1, p2 }, color );
			}

			for ( const auto point : box.points() )
			{
				drawPointText( point, color );
			}
		}

		inline void drawLine( const Line< CoordinateSpace::World > line, const glm::vec3 color, const float thickness )
		{
			const Coordinate< CoordinateSpace::Screen > start_screen { toScreenSpace( line.start ) };
			const Coordinate< CoordinateSpace::Screen > end_screen { toScreenSpace( line.end ) };

			if ( !inView( start_screen ) && !inView( end_screen ) ) return;
			if ( isBehind( start_screen ) || isBehind( end_screen ) ) return;

			ImGui::GetForegroundDrawList()->AddLine(
				glmToImgui( start_screen ), glmToImgui( end_screen ), ImColor( color.x, color.y, color.z ), thickness );
		}

		void drawLine(
			const Coordinate< CoordinateSpace::World > start,
			const Coordinate< CoordinateSpace::World > end,
			const glm::vec3 color )
		{
			drawLine( { start, end }, color );
		}

		void drawPointText( const Coordinate< CoordinateSpace::World > point, const glm::vec3 color )
		{
			const glm::vec3 screen_point { toScreenSpace( point ) };

			if ( !inView( screen_point ) ) return;

			drawPoint( point, "", color );

			const std::string text { "World: (" + std::to_string( point.x ) + ", " + std::to_string( point.y ) + ", "
				                     + std::to_string( point.z ) + ")" };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y ), text, color, glm::vec2( 0.0f, 20.f ) );

			const std::string text2 { "Screen: (" + std::to_string( screen_point.x ) + ", "
				                      + std::to_string( screen_point.y ) + ", " + std::to_string( screen_point.z )
				                      + ")" };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y ), text2, color, glm::vec2( 0.0f, 30.0f ) );

			const Frustum frustum { getDebugDrawingCamera().getFrustumBounds() };
			const bool in_view { frustum.pointInside( point ) };

			drawBoolAlpha( point, in_view, glm::vec2( 0.0f, 40.0f ) );
		}

		void drawPointLabel( const Coordinate< CoordinateSpace::World > point, const std::string label )
		{
			const glm::vec3 screen_point { toScreenSpace( point ) };

			if ( !inView( screen_point ) ) return;

			screen::drawText( glm::vec2( screen_point.x, screen_point.y ), label );
		}

		void drawPoint(
			const Coordinate< CoordinateSpace::World > point, const std::string label, const glm::vec3 color )
		{
			const auto screen_point { toScreenSpace( point ) };
			if ( !inView( screen_point ) ) return;

			const float div { screen_point.z };

			ImGui::GetForegroundDrawList()
				->AddCircleFilled( glmToImgui( screen_point ), div * 5.0f, ImColor( color.x, color.y, color.z ) );

			drawPointLabel( point, label );
		}

		void drawBoolAlpha( const Coordinate< CoordinateSpace::World > point, const bool value, const glm::vec2 offset )
		{
			const auto screen_point { toScreenSpace( point ) };

			const auto color { value ? glm::vec3( 0.0f, 1.0f, 0.0f ) : glm::vec3( 1.0f, 0.0f, 0.0f ) };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y + offset.y ), value ? "true" : "false", color );
		}

		void drawVector(
			const Coordinate< CoordinateSpace::World > point,
			Vector vector,
			const std::string label,
			const glm::vec3 color )
		{
			drawLine( point, point + glm::normalize( vector ), color );
			drawPoint( point + glm::normalize( vector ), label, color );
			drawPoint( point, "", color );
			//drawPointLabel( point, label );
			//drawPointText( point + glm::normalize( vector ) );

			//Draw ending lines for the vector (two perpendicular lines)
			const WorldCoordinate perpendicular_vector {
				glm::normalize( glm::cross( vector, glm::vec3( 0.0f, 1.0f, 0.0f ) ) ) / 4.0f
			};
			const WorldCoordinate perpendicular_vector2 { glm::normalize( glm::cross( vector, perpendicular_vector ) )
				                                          / 4.0f };

			drawLine(
				point + glm::normalize( vector ) + perpendicular_vector,
				point + glm::normalize( vector ) - perpendicular_vector,
				color );

			drawLine(
				point + glm::normalize( vector ) + perpendicular_vector2,
				point + glm::normalize( vector ) - perpendicular_vector2,
				color );
		}

		void drawFrustum(
			const Frustum< CoordinateSpace::World >& frustum, [[maybe_unused]] const WorldCoordinate point )
		{
			drawPlane( frustum.near, frustum.near.getPosition(), "near" );
			drawPlane( frustum.far, frustum.far.getPosition(), "far" );
			drawPlane( frustum.top, frustum.top.getPosition(), "top" );
			drawPlane( frustum.bottom, frustum.bottom.getPosition(), "bottom" );
			drawPlane( frustum.right, frustum.right.getPosition(), "right" );
			drawPlane( frustum.left, frustum.left.getPosition(), "left" );
		}

		void drawFrustum()
		{
			const Frustum frustum { getDebugDrawingCamera().getFrustumBounds() };
			drawFrustum( frustum, getDebugDrawingCamera().getFrustumPosition() );
		}

		void drawPlane(
			const Plane< CoordinateSpace::World >& plane,
			const WorldCoordinate point,
			const std::string label,
			const glm::vec3 color )
		{
			const auto normal { plane.direction() };

			assert( point != constants::DEFAULT_VEC3 );

			drawLine( point, point + normal, color );
			drawPoint( point + normal, label, color );
		}

	} // namespace world

	namespace screen
	{
		void
			drawText( const glm::vec2 position, const std::string& text, const glm::vec3 color, const glm::vec2 offset )
		{
			ImGui::GetForegroundDrawList()
				->AddText( glmToImgui( position + offset ), ImColor( color.x, color.y, color.z ), text.c_str() );
		}

		void drawBoolAlpha(
			const glm::vec2 screen_point,
			[[maybe_unused]] const Camera& camera,
			const bool value,
			const glm::vec2 offset )
		{
			const auto color { value ? glm::vec3( 0.0f, 1.0f, 0.0f ) : glm::vec3( 1.0f, 0.0f, 0.0f ) };
			drawText( glm::vec2( screen_point.x, screen_point.y + offset.y ), value ? "true" : "false", color );
		}

	} // namespace screen

} // namespace fgl::engine::debug

#endif
