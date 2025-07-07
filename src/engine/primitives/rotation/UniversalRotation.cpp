//
// Created by kj16609 on 2/27/25.
//
#include "UniversalRotation.hpp"

namespace fgl::engine
{

	void UniversalRotation::addX( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addX( value );
		else [[unlikely]]
			m_euler_rot.x += value;
	}

	void UniversalRotation::addY( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addY( value );
		else [[unlikely]]
			m_euler_rot.y += value;
	}

	void UniversalRotation::addZ( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addZ( value );
		else [[unlikely]]
			m_euler_rot.z += value;
	}

	void UniversalRotation::addXWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addXWorld( value );
		else [[unlikely]]
			m_euler_rot.x += value;
	}

	void UniversalRotation::addYWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addYWorld( value );
		else [[unlikely]]
			m_euler_rot.y += value;
	}

	void UniversalRotation::addZWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			m_quat_rot.addZWorld( value );
		else [[unlikely]]
			m_euler_rot.z += value;
	}

} // namespace fgl::engine