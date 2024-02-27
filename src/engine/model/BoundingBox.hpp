//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <glm/vec3.hpp>
#include <tracy/Tracy.hpp>

#include <array>
#include <vector>

#include "engine/constants.hpp"
#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Line.hpp"
#include "engine/primitives/Matrix.hpp"

namespace fgl::engine
{
	template < CoordinateSpace type >
	struct Frustum;

	struct Vertex;

	template < CoordinateSpace CType >
	struct BoundingBox
	{
		Coordinate< CType > middle { constants::DEFAULT_VEC3 };
		glm::vec3 scale { 0.0f };

		//! Returns the top left (-x, -y, -z) coordinate
		inline glm::vec3 bottomLeftBack() const { return middle - scale; }

		//! Returns the bottom right (x, y, z) coordinate
		inline glm::vec3 topRightForward() const { return middle + scale; }

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		std::vector< Coordinate< CType > > points() const;
		std::vector< Line< CType > > lines() const;

		bool isInFrustum( const Frustum< CType >& frustum ) const;

		BoundingBox combine( const BoundingBox& other ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	BoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const BoundingBox< CType > bounding_box )
	{
		const Coordinate< EvolvedType< MType >() > new_middle { matrix * bounding_box.middle };
		const glm::vec3 new_scale { matrix * glm::vec4( bounding_box.scale, 0.0f ) };
		return BoundingBox< EvolvedType< MType >() >( new_middle, new_scale );
	}

	template < CoordinateSpace CType >
	BoundingBox< CType > generateBoundingFromPoints( const std::vector< Coordinate< CType > >& points );

	using ModelBoundingBox = BoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
