//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <glm/vec3.hpp>

#include <array>
#include <vector>

#include "BoundingBox.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/Rotation.hpp"
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
		{}

	  private:

		//! Returns the top left (-x, -y, -z) coordinate
		inline glm::vec3 bottomLeftBack() const { return middle.vec() - scale; }

		//! Returns the bottom right (x, y, z) coordinate
		inline glm::vec3 topRightForward() const { return middle.vec() + scale; }

	  public:

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		std::vector< Coordinate< CType > > points() const;
		std::vector< LineSegment< CType > > lines() const;

		NormalVector forward() const { return rotation.forward(); }

		NormalVector right() const { return rotation.right(); }

		NormalVector up() const { return rotation.up(); }

		OrientedBoundingBox combine( const OrientedBoundingBox& other ) const;
	};

	template < CoordinateSpace CType, MatrixType MType >
	OrientedBoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const OrientedBoundingBox< CType > bounding_box )
	{
		const Coordinate< EvolvedType< MType >() > new_middle { matrix * bounding_box.middle };
		const glm::vec3 new_scale { matrix * glm::vec4( bounding_box.scale, 0.0f ) };
		//TODO: Fix this stupid rot shit
		const Rotation new_rot { bounding_box.rotation };
		return OrientedBoundingBox< EvolvedType< MType >() >( new_middle, new_scale, new_rot );
	}

	template < CoordinateSpace CType >
	OrientedBoundingBox< CType > generateBoundingFromPoints( const std::vector< Coordinate< CType > >& points );

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< Vertex >& points );

	using ModelBoundingBox = OrientedBoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
