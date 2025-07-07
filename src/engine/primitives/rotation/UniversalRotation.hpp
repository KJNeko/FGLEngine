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
			QuatRotation m_quat_rot;
			EulerRotation m_euler_rot { constants::DEFAULT_ROTATION };
		};

		//! If true then the rotation is in a
		bool m_euler { true };

	  public:

		[[nodiscard]] UniversalRotation() : m_euler_rot( constants::DEFAULT_ROTATION ), m_euler( true ) {}

		[[nodiscard]] UniversalRotation( const EulerRotation& euler ) : m_euler_rot( euler ) {}

		[[nodiscard]] UniversalRotation( const QuatRotation& q_rotation ) : m_quat_rot( q_rotation ) {}

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
				return m_euler_rot.toRotation();
			else [[likely]]
				return m_quat_rot;
		}

		// Marked cold because it's only usd in the editor
		FGL_COLD EulerRotation& euler()
		{
			FGL_ASSERT( isEuler(), "Rotation is not euler" );
			return m_euler_rot;
		}

		// Marked cold because it's only usd in the editor
		FGL_COLD const EulerRotation& euler() const
		{
			FGL_ASSERT( isEuler(), "Rotation is not euler" );
			return m_euler_rot;
		}

		FGL_HOT QuatRotation& quat()
		{
			FGL_ASSERT( isQuat(), "Rotation is not quat" );
			return m_quat_rot;
		}

		FGL_HOT const QuatRotation& quat() const
		{
			FGL_ASSERT( isQuat(), "Rotation is not quat" );
			return m_quat_rot;
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