//
// Created by kj16609 on 1/27/24.
//

#pragma once

#include <array>
#include <vector>

#include "BoundingBox.hpp"
#include "engine/constants.hpp"
#include "engine/primitives/Scale.hpp"
#include "engine/primitives/TransformComponent.hpp"
#include "engine/primitives/matricies/Matrix.hpp"
#include "engine/primitives/points/Coordinate.hpp"

namespace fgl::engine
{
	struct Frustum;

	template < CoordinateSpace CType >
	class LineSegment;

	template < CoordinateSpace CType >
	class AxisAlignedBoundingBox;

	struct ModelVertex;

	template < CoordinateSpace CType >
	struct OrientedBoundingBox final : public interface::BoundingBox
	{
		TransformComponent< CType > m_transform;

		OrientedBoundingBox() : m_transform( Coordinate< CType >( constants::DEFAULT_VEC3 ) ) {}

		OrientedBoundingBox( const Coordinate< CType > pos, const glm::vec3 inital_scale ) :
		  m_transform( pos, Scale( inital_scale ) )
		{
			assert( pos.vec() != constants::DEFAULT_VEC3 );
			assert( inital_scale != constants::DEFAULT_VEC3 );
		}

		OrientedBoundingBox( const TransformComponent< CType >& transform ) : m_transform( transform )
		{
			assert( m_transform.translation.vec() != constants::DEFAULT_VEC3 );
			assert( m_transform.scale != constants::DEFAULT_VEC3 );
		}

	  public:

		//! Returns the top left (-x, -y, -z) coordinate
		Coordinate< CType > bottomLeftBack() const
		{
			return m_transform.translation - Scale( glm::abs( static_cast< glm::vec3 >( m_transform.scale ) ) );
		}

		//! Returns the bottom right (x, y, z) coordinate
		Coordinate< CType > topRightForward() const
		{
			return m_transform.translation + Scale( glm::abs( static_cast< glm::vec3 >( m_transform.scale ) ) );
		}

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		std::array< Coordinate< CType >, POINT_COUNT > points() const;
		std::array< LineSegment< CType >, LINE_COUNT > lines() const;

		NormalVector forward() const { return m_transform.forward(); }

		NormalVector right() const { return m_transform.right(); }

		NormalVector up() const { return m_transform.up(); }

		Coordinate< CType > center() { return m_transform.translation; }

		OrientedBoundingBox combine( const OrientedBoundingBox& other ) const;

		AxisAlignedBoundingBox< CType > alignToWorld();
	};

	template < CoordinateSpace CType, MatrixType MType >
	OrientedBoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const OrientedBoundingBox< CType > bounding_box )
	{
		assert( bounding_box.m_transform.translation.vec() != constants::DEFAULT_VEC3 );
		assert( bounding_box.m_transform.scale != glm::vec3( 0.0f ) );

		const TransformComponent< EvolvedType< MType >() > new_transform { matrix * bounding_box.m_transform };

		assert( bounding_box.m_transform.translation.vec() != constants::DEFAULT_VEC3 );
		assert( bounding_box.m_transform.scale != glm::vec3( 0.0f ) );

		return { new_transform };
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< ModelVertex >& verts );

	using ModelBoundingBox = OrientedBoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
