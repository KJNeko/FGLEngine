//
// Created by kj16609 on 2/27/25.
//
#include "UniversalRotation.hpp"

namespace fgl::engine
{

	void UniversalRotation::addX( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addX( value );
		else [[unlikely]]
			e_rotation.x += value;
	}

	void UniversalRotation::addY( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addY( value );
		else [[unlikely]]
			e_rotation.y += value;
	}

	void UniversalRotation::addZ( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addZ( value );
		else [[unlikely]]
			e_rotation.z += value;
	}

	void UniversalRotation::addXWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addXWorld( value );
		else [[unlikely]]
			e_rotation.x += value;
	}

	void UniversalRotation::addYWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addYWorld( value );
		else [[unlikely]]
			e_rotation.y += value;
	}

	void UniversalRotation::addZWorld( const float value )
	{
		if ( isQuat() ) [[likely]]
			q_rotation.addZWorld( value );
		else [[unlikely]]
			e_rotation.z += value;
	}

} // namespace fgl::engine