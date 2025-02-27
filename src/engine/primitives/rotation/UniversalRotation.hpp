//
// Created by kj16609 on 2/27/25.
//
#pragma once
#include "EulerRotation.hpp"
#include "FGL_DEFINES.hpp"
#include "QuatRotation.hpp"

namespace fgl::engine
{
	class UniversalRotation
	{
		union
		{
			QuatRotation q_rotation;
			EulerRotation e_rotation { constants::DEFAULT_ROTATION };
		};

		//! If true then the rotation is in a
		bool m_euler { true };

	  public:

		[[nodiscard]] UniversalRotation() : e_rotation( constants::DEFAULT_ROTATION ), m_euler( true ) {}

		[[nodiscard]] UniversalRotation( const EulerRotation& euler ) : e_rotation( euler ) {}

		[[nodiscard]] UniversalRotation( const QuatRotation& q_rotation ) : q_rotation( q_rotation ) {}

		FGL_DEFAULT_MOVE( UniversalRotation );
		FGL_DEFAULT_COPY( UniversalRotation );

		NormalVector forward() const { return forcedQuat().forward(); }

		NormalVector right() const { return forcedQuat().right(); }

		NormalVector up() const { return forcedQuat().up(); }

		inline bool isEuler() const { return m_euler; }

		inline bool isQuat() const { return !isEuler(); }

		//! Returns a quaternion rotation no matter the original rotation
		FGL_HOT [[nodiscard]] QuatRotation forcedQuat() const
		{
			if ( m_euler ) [[unlikely]]
				return e_rotation.toRotation();
			else [[likely]]
				return q_rotation;
		}

		// Marked cold because it's only usd in the editor
		FGL_COLD EulerRotation& euler()
		{
			FGL_ASSERT( isEuler(), "Rotation is not euler" );
			return e_rotation;
		}

		// Marked cold because it's only usd in the editor
		FGL_COLD const EulerRotation& euler() const
		{
			FGL_ASSERT( isEuler(), "Rotation is not euler" );
			return e_rotation;
		}

		FGL_HOT QuatRotation& quat()
		{
			FGL_ASSERT( isQuat(), "Rotation is not quat" );
			return q_rotation;
		}

		FGL_HOT const QuatRotation& quat() const
		{
			FGL_ASSERT( isQuat(), "Rotation is not quat" );
			return q_rotation;
		}

		// Universal modification
		void addX( float value );
		void addY( float value );
		void addZ( float value );

		void addXWorld( float value );
		void addYWorld( float value );
		void addZWorld( float value );
	};
} // namespace fgl::engine