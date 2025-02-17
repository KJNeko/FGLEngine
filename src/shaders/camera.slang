#version 450

module camera;

public struct CameraData
{
	public mat4x4 projection : CAMERA_PROJECTION;
	public mat4x4 view : CAMERA_VIEW;
	public mat4x4 inverse_view : CAMERA_INVERSE_VIEW;

	public vec3 getPos()
	{
	    //TODO: For some reason even though the compiler is set to be column major, This array access is inverted. Need to look into this.
		return vec3( inverse_view[ 0 ][ 3 ], inverse_view[ 1 ][ 3 ], inverse_view[ 2 ][ 3 ] );
	}

}