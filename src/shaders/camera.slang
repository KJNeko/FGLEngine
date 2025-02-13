#version 450

module camera;

public struct CameraData
{
	public mat4x4 projection : CAMERA_PROJECTION;
	public mat4x4 view : CAMERA_VIEW;
	public mat4x4 inverse_view : CAMERA_INVERSE_VIEW;

	public vec3 getPos()
	{
		return vec3( inverse_view[ 3 ][ 0 ], inverse_view[ 3 ][ 1 ], inverse_view[ 3 ][ 2 ] );
	}

}