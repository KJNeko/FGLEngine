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
	class OrientedBoundingBox final : public interface::BoundingBox
	{
	  public:

		enum class TransformMode : std::uint8_t
		{
			Transform,
			Matrix
		};

	  private:

		TransformMode transform_mode;

		union
		{
			TransformComponent< CType > m_transform;
			glm::mat4 m_matrix;
		};

		friend class AxisAlignedBoundingBox< CType >;

	  public:

		[[nodiscard]] OrientedBoundingBox() :
		  transform_mode( TransformMode::Transform ),
		  m_transform( Coordinate< CType >( constants::DEFAULT_VEC3 ) )
		{}

		[[nodiscard]] OrientedBoundingBox( const Coordinate< CType > pos, const glm::vec3 inital_scale ) :
		  transform_mode( TransformMode::Transform ),
		  m_transform( pos, Scale( inital_scale ) )
		{
			assert( pos.vec() != constants::DEFAULT_VEC3 );
			assert( inital_scale != constants::DEFAULT_VEC3 );
		}

		[[nodiscard]] OrientedBoundingBox( const TransformComponent< CType >& transform ) :
		  transform_mode( TransformMode::Transform ),
		  m_transform( transform )
		{
			assert( m_transform.translation.vec() != constants::DEFAULT_VEC3 );
			assert( m_transform.scale != constants::DEFAULT_VEC3 );
		}

		[[nodiscard]] OrientedBoundingBox( const glm::mat4 matrix ) :
		  transform_mode( TransformMode::Matrix ),
		  m_matrix( matrix )
		{}

		//! Returns the top left (-x, -y, -z) coordinate
		Coordinate< CType > bottomLeftBack() const
		{
			switch ( transform_mode )
			{
				case TransformMode::Transform:
					return m_transform.translation - Scale( glm::abs( static_cast< glm::vec3 >( m_transform.scale ) ) );
				case TransformMode::Matrix:
					return Coordinate< CType >( m_matrix * glm::vec4( -1.0f, -1.0f, -1.0f, 1.0f ) );
				default:
					FGL_UNREACHABLE();
			}
			FGL_UNREACHABLE();
		}

		//! Returns the bottom right (x, y, z) coordinate
		[[nodiscard]] Coordinate< CType > topRightForward() const
		{
			switch ( transform_mode )
			{
				case TransformMode::Transform:
					return m_transform.translation + Scale( glm::abs( static_cast< glm::vec3 >( m_transform.scale ) ) );
				case TransformMode::Matrix:
					return Coordinate< CType >( m_matrix * glm::vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
				default:
					FGL_UNREACHABLE();
			}
			FGL_UNREACHABLE();
		}

		// 6 sides, 2 triangles each, 3 verts per triangle
		constexpr static std::uint32_t indicies_count { 6 * 2 * 3 };

		[[nodiscard]] consteval static std::array< std::uint32_t, indicies_count > triangleIndicies();

		[[nodiscard]] std::array< Coordinate< CType >, POINT_COUNT > points() const;
		[[nodiscard]] std::array< LineSegment< CType >, LINE_COUNT > lines() const;

		[[nodiscard]] NormalVector forward() const { return m_transform.forward(); }

		[[nodiscard]] NormalVector right() const { return m_transform.right(); }

		[[nodiscard]] NormalVector up() const { return m_transform.up(); }

		[[nodiscard]] Coordinate< CType > center() { return m_transform.translation; }

		[[nodiscard]] OrientedBoundingBox combine( const OrientedBoundingBox& other ) const;

		[[nodiscard]] AxisAlignedBoundingBox< CType > alignToWorld() const;

		[[nodiscard]] Coordinate< CType > getPosition() const;

		TransformMode getMode() const { return transform_mode; }

		[[nodiscard]] TransformComponent< CType >& getTransform()
		{
			FGL_ASSERT(
				transform_mode == TransformMode::Transform, "FIXME: Transform not initalized. Mode switch needed" );
			return m_transform;
		}

		[[nodiscard]] const TransformComponent< CType >& getTransform() const
		{
			FGL_ASSERT(
				transform_mode == TransformMode::Transform, "FIXME: Transform not initalized. Mode switch needed" );
			return m_transform;
		}

		[[nodiscard]] glm::mat4 getMatrix() const
		{
			switch ( transform_mode )
			{
				case TransformMode::Transform:
					return m_transform.mat4();
				case TransformMode::Matrix:
					return m_matrix;
				default:
					FGL_UNREACHABLE();
			}
		}
	};

	template < CoordinateSpace CType, MatrixType MType >
	OrientedBoundingBox< EvolvedType< MType >() >
		operator*( const Matrix< MType > matrix, const OrientedBoundingBox< CType > bounding_box )
	{
		assert( bounding_box.getTransform().translation.vec() != constants::DEFAULT_VEC3 );
		assert( bounding_box.getTransform().scale != glm::vec3( 0.0f ) );

		switch ( bounding_box.getMode() )
		{
			case OrientedBoundingBox< CType >::TransformMode::Transform:
				{
					const auto new_matrix { matrix * bounding_box.getTransform().mat() };
					return { new_matrix };
				}
			case OrientedBoundingBox< CType >::TransformMode::Matrix:
				{
					const auto new_matrix { matrix * bounding_box.getMatrix() };
					return { new_matrix };
				}
			default:
				FGL_UNREACHABLE();
		}
	}

	OrientedBoundingBox< CoordinateSpace::Model > generateBoundingFromVerts( const std::vector< ModelVertex >& verts );

	using ModelBoundingBox = OrientedBoundingBox< CoordinateSpace::Model >;

} // namespace fgl::engine
