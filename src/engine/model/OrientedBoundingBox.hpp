//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <glm/vec3.hpp>

#include <array>
#include <vector>

#include "engine/constants.hpp"
#include "engine/primitives/Coordinate.hpp"
#include "engine/primitives/Matrix.hpp"
#include "engine/primitives/Rotation.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct Frustum;

	template < CoordinateSpace CType >
	struct Line;

	struct Vertex;

	template < CoordinateSpace CType >
	struct OrientedBoundingBox
	{
		Coordinate< CType > middle;
		glm::vec3 scale;
		Rotation rotation;

		OrientedBoundingBox() : middle( constants::DEFAULT_VEC3 ), scale( 0.0f ), rotation( 0.0f, 0.0f, 0.0f ) {}

		OrientedBoundingBox(
			const Coordinate< CType > pos, glm::vec3 inital_scale, const Rotation inital_rotation = {} ) :
		  middle( pos ),
		  scale( inital_scale ),
		  rotation( inital_rotation )
		{}

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

		OrientedBoundingBox combine( const OrientedBoundingBox& other ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	OrientedBoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const OrientedBoundingBox< CType > bounding_box )
	{
		const Coordinate< EvolvedType< MType >() > new_middle { matrix * bounding_box.middle };
		const glm::vec3 new_scale { matrix * glm::vec4( bounding_box.scale, 0.0f ) };
		return OrientedBoundingBox< EvolvedType< MType >() >( new_middle, new_scale );
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > generateBoundingFromPoints( const std::vector< Coordinate< CType > >& points );

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< Vertex >& points );

	using ModelBoundingBox = OrientedBoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
