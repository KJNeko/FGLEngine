//
// Created by kj16609 on 1/28/24.
//

#include "drawers.hpp"

#include "engine/Camera.hpp"
#include "engine/model/BoundingBox.hpp"
#include "engine/primitives/Line.hpp"
#include "engine/primitives/Vector.hpp"

namespace fgl::engine::debug
{
	const ImVec2 windowSize()
	{
		return ImGui::GetMainViewport()->Size;
	}

	Coordinate< CoordinateSpace::Screen >
		toScreenSpace( Coordinate< CoordinateSpace::World > world_point, const Camera& camera )
	{
		ZoneScoped;
		const ImVec2 window_size { windowSize() };

		world_point.z = -world_point.z;

		return Coordinate< CoordinateSpace::Screen >( glm::project(
			static_cast< glm::vec3 >( world_point ),
			glm::mat4( 1.0f ),
			camera.getProjectionViewMatrix(),
			glm::vec4( 0.0f, 0.0f, window_size.x, window_size.y ) ) );
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
		void drawBoundingBox( const BoundingBox< CoordinateSpace::World >& box, Camera& camera, const glm::vec3 color )
		{
			ZoneScoped;
			for ( const auto [ p1, p2 ] : box.lines() )
			{
				drawLine( p1, p2, camera, color );
			}

			for ( const auto point : box.points() )
			{
				drawPointText( point, camera, color );
			}
		}

		void drawLine( const Line< CoordinateSpace::World > line, const Camera& camera, const glm::vec3 color )
		{
			ZoneScoped;
			const Coordinate< CoordinateSpace::Screen > start_screen { toScreenSpace( line.start, camera ) };
			const Coordinate< CoordinateSpace::Screen > end_screen { toScreenSpace( line.end, camera ) };

			if ( !inView( start_screen ) && !inView( end_screen ) ) return;

			if ( isBehind( start_screen ) || isBehind( end_screen ) ) return;

			ImGui::GetForegroundDrawList()
				->AddLine( glmToImgui( start_screen ), glmToImgui( end_screen ), ImColor( color.x, color.y, color.z ) );
		}

		void drawLine(
			const Coordinate< CoordinateSpace::World > start,
			const Coordinate< CoordinateSpace::World > end,
			const Camera& camera,
			const glm::vec3 color )
		{
			drawLine( { start, end }, camera, color );
		}

		void drawPointText(
			const Coordinate< CoordinateSpace::World > point, const Camera& camera, const glm::vec3 color )
		{
			ZoneScoped;
			const glm::vec3 screen_point { toScreenSpace( point, camera ) };

			if ( !inView( screen_point ) ) return;

			drawPoint( point, camera, color );

			const std::string text { "World: (" + std::to_string( point.x ) + ", " + std::to_string( point.y ) + ", "
				                     + std::to_string( point.z ) + ")" };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y ), text, color, glm::vec2( 0.0f, 20.f ) );

			const std::string text2 { "Screen: (" + std::to_string( screen_point.x ) + ", "
				                      + std::to_string( screen_point.y ) + ", " + std::to_string( screen_point.z )
				                      + ")" };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y ), text2, color, glm::vec2( 0.0f, 30.0f ) );

			const Frustum frustum { camera.getFrustumBounds() };
			const bool in_view { frustum.pointInside( point ) };

			drawBoolAlpha( point, camera, in_view, glm::vec2( 0.0f, 40.0f ) );
		}

		void drawPoint( const Coordinate< CoordinateSpace::World > point, const Camera& camera, const glm::vec3 color )
		{
			ZoneScoped;
			const auto screen_point { toScreenSpace( point, camera ) };
			if ( !inView( screen_point ) ) return;

			ImGui::GetForegroundDrawList()
				->AddCircleFilled( glmToImgui( screen_point ), 5.0f, ImColor( color.x, color.y, color.z ) );
		}

		void drawBoolAlpha(
			const Coordinate< CoordinateSpace::World > point,
			const Camera& camera,
			const bool value,
			const glm::vec2 offset )
		{
			ZoneScoped;
			const auto screen_point { toScreenSpace( point, camera ) };

			const auto color { value ? glm::vec3( 0.0f, 1.0f, 0.0f ) : glm::vec3( 1.0f, 0.0f, 0.0f ) };

			screen::drawText( glm::vec2( screen_point.x, screen_point.y + offset.y ), value ? "true" : "false", color );
		}

		void drawVector(
			const Coordinate< CoordinateSpace::World > point,
			const Vector vector,
			const Camera& camera,
			const glm::vec3 color )
		{
			drawLine( point, point + vector.norm(), camera, color );
			drawPoint( point + vector.norm(), camera, color );

			//Draw ending lines for the vector (two perpendicular lines)
			const glm::vec3 perpendicular_vector { glm::normalize( glm::cross( vector, glm::vec3( 0.0f, 1.0f, 0.0f ) ) )
				                                   / 4.0f };
			const glm::vec3 perpendicular_vector2 { glm::normalize( glm::cross( vector, perpendicular_vector ) )
				                                    / 4.0f };

			drawLine(
				point + vector.norm() + perpendicular_vector,
				point + vector.norm() - perpendicular_vector,
				camera,
				color );

			drawLine(
				point + vector.norm() + perpendicular_vector2,
				point + vector.norm() - perpendicular_vector2,
				camera,
				color );
		}

		void drawFrustum( const Frustum< CoordinateSpace::World >& frustum, const Camera& camera )
		{
			drawPlane( frustum.near, camera );
			drawPlane( frustum.far, camera );
			drawPlane( frustum.top, camera );
			drawPlane( frustum.bottom, camera );
			drawPlane( frustum.right, camera );
			drawPlane( frustum.left, camera );
		}

		void drawCameraInfo( const Camera& camera )
		{
			/*
			drawVector( { 0.0f, 0.0f, 0.0f }, camera.getUp(), camera, glm::vec3( 0.0f, 1.0f, 0.0f ) );
			drawVector( { 0.0f, 0.0f, 0.0f }, camera.getRight(), camera, glm::vec3( 1.0f, 0.0f, 0.0f ) );
			drawVector( { 0.0f, 0.0f, 0.0f }, camera.getForward(), camera, glm::vec3( 0.0f, 0.0f, 1.0f ) );
			 */

			{
				const Frustum frustum { camera.getFrustumBounds() };

				drawFrustum( frustum, camera );
			}

			/*
			{
				const Frustum frustum { camera.getFrustumBounds() };
				const auto pos { frustum.near.getPosition() };

				drawVector( pos, frustum.far.direction(), camera );
				drawVector( pos, frustum.near.direction(), camera );
				drawVector( pos, frustum.top.direction(), camera );
				drawVector( pos, frustum.bottom.direction(), camera );
				drawVector( pos, frustum.right.direction(), camera );
				drawVector( pos, frustum.left.direction(), camera );
			}
			*/
		}

		void drawPlane( const Plane< CoordinateSpace::World >& plane, const Camera& camera, const glm::vec3 color )
		{
			assert( plane.direction() != constants::DEFAULT_VEC3 );
			assert( plane.getPosition() != constants::DEFAULT_VEC3 );

			const auto pos { plane.getPosition() };
			const auto dir { plane.direction() };

			drawVector( pos, dir, camera, color );
		}

	} // namespace world

	namespace screen
	{
		void
			drawText( const glm::vec2 position, const std::string& text, const glm::vec3 color, const glm::vec2 offset )
		{
			ZoneScoped;
			ImGui::GetForegroundDrawList()
				->AddText( glmToImgui( position + offset ), ImColor( color.x, color.y, color.z ), text.c_str() );
		}

		void drawBoolAlpha(
			const glm::vec2 screen_point, const Camera& camera, const bool value, const glm::vec2 offset )
		{
			ZoneScoped;
			const auto color { value ? glm::vec3( 0.0f, 1.0f, 0.0f ) : glm::vec3( 1.0f, 0.0f, 0.0f ) };
			drawText( glm::vec2( screen_point.x, screen_point.y + offset.y ), value ? "true" : "false", color );
		}

	} // namespace screen

} // namespace fgl::engine::debug
