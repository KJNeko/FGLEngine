//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <array>
#include <vector>

#include "BoundingBox.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/Rotation.hpp"
#include "engine/primitives/Scale.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	template < CoordinateSpace CType >
	struct Frustum;

	template < CoordinateSpace CType >
	class LineSegment;

	struct Vertex;

	template < CoordinateSpace CType >
	struct OrientedBoundingBox : public interface::BoundingBox
	{
		Coordinate< CType > middle;
		glm::vec3 scale;
		Rotation rotation;

		OrientedBoundingBox() : middle( constants::DEFAULT_VEC3 ), scale( 0.0f ), rotation() {}

		OrientedBoundingBox(
			const Coordinate< CType > pos, glm::vec3 inital_scale, const Rotation inital_rotation = {} ) :
		  middle( pos ),
		  scale( inital_scale ),
		  rotation( inital_rotation )
		{
			assert( pos.vec() != constants::DEFAULT_VEC3 );
			assert( inital_scale != constants::DEFAULT_VEC3 );
		}

	  public:

		//! Returns the top left (-x, -y, -z) coordinate
		inline Coordinate< CType > bottomLeftBack() const { return middle - Scale( glm::abs( scale ) ); }

		//! Returns the bottom right (x, y, z) coordinate
		inline Coordinate< CType > topRightForward() const { return middle + Scale( glm::abs( scale ) ); }

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		std::array< Coordinate< CType >, POINT_COUNT > points() const;
		std::array< LineSegment< CType >, LINE_COUNT > lines() const;

		NormalVector forward() const { return rotation.forward(); }

		NormalVector right() const { return rotation.right(); }

		NormalVector up() const { return rotation.up(); }

		OrientedBoundingBox combine( const OrientedBoundingBox& other ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	OrientedBoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const OrientedBoundingBox< CType > bounding_box )
	{
		assert( bounding_box.middle.vec() != constants::DEFAULT_VEC3 );
		assert( bounding_box.scale != glm::vec3( 0.0f ) );

		const Coordinate< EvolvedType< MType >() > new_middle { matrix * bounding_box.middle };
		const glm::vec3 new_scale { matrix * glm::vec4( bounding_box.scale, 0.0f ) };
		//TODO: Fix this stupid rot shit
		const Rotation new_rot { bounding_box.rotation };

		return OrientedBoundingBox< EvolvedType< MType >() >( new_middle, new_scale, new_rot );
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< Vertex >& verts );

	using ModelBoundingBox = OrientedBoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
