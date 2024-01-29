//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <glm/vec3.hpp>

#include <array>
#include <vector>

#include "engine/coordinates/WorldCoordinate.hpp"

namespace fgl::engine
{
	class Frustum;

	struct BoundingBox
	{
		glm::vec3 middle { DEFAULT_COORDINATE_VEC3 };
		glm::vec3 scale { 0.0f };

		//! Returns the top left (-x, -y, -z) coordinate
		inline glm::vec3 topLeftFront() const { return middle - scale; }

		//! Returns the bottom right (x, y, z) coordinate
		inline glm::vec3 bottomRightBack() const { return middle + scale; }

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		std::vector< glm::vec3 > points() const;
		std::vector< std::pair< glm::vec3, glm::vec3 > > lines() const;

		bool isInFrustum( const Frustum& frustum ) const;

		BoundingBox combine( const BoundingBox& other ) const;

		BoundingBox operator*( glm::mat4 matrix ) const;
	};

	BoundingBox generateBoundingFromPoints( std::vector< glm::vec3 >& points );

} // namespace fgl::engine
