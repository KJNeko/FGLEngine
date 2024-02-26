//
// Created by kj16609 on 2/19/24.
//

#include "taitBryanMatrix.hpp"

#include <glm/mat4x4.hpp>

#include <stdexcept>
#include <tuple>
#include <utility>

#include "engine/primitives/Rotation.hpp"

namespace fgl::engine
{

	template < int N >
	inline std::tuple< float, float > extract( const Rotation rotation, const RotationOrder order )
	{
		switch ( order )
		{
			case XZY:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 2:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 3:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
				}
				break;
			case XYZ: // DEFAULT
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 2:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 3:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
				}
				break;
			case YXZ:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 2:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 3:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
				}
				break;
			case YZX:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 2:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 3:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
				}
				break;
			case ZYX:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 2:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
					case 3:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
				}
				break;
			case ZXY:
				switch ( N )
				{
					case 1:
						return { glm::cos( rotation.z ), glm::sin( rotation.z ) };
					case 2:
						return { glm::cos( rotation.x ), glm::sin( rotation.x ) };
					case 3:
						return { glm::cos( rotation.y ), glm::sin( rotation.y ) };
				}
				break;
			case END_OF_ENUM:
				throw std::runtime_error( "Unimplemented rotation order" );
		}
		std::unreachable();
	}

	glm::mat4 taitBryanMatrix( const Rotation rotation, const RotationOrder order )
	{
		glm::mat4 mat { 1.0f };

		//TODO: Debug with Entry, There has got to be a better fix then this.
		Rotation fixed_rotation { rotation };
		fixed_rotation.pitch() = -fixed_rotation.pitch();
		fixed_rotation.roll() = -fixed_rotation.roll();

		const auto [ c1, s1 ] = extract< 1 >( fixed_rotation, order );
		const auto [ c2, s2 ] = extract< 2 >( fixed_rotation, order );
		const auto [ c3, s3 ] = extract< 3 >( fixed_rotation, order );

		switch ( order )
		{
			case RotationOrder::XZY:
				{
					const glm::vec3 row_0 { ( c2 * c3 ), -( s2 ), ( c2 * s3 ) };

					const glm::vec3 row_1 { ( s1 * s3 ) + ( c1 * c3 * s2 ),
						                    ( c1 * c2 ),
						                    ( c1 * s2 * s3 ) - ( c3 * s1 ) };

					const glm::vec3 row_2 { ( c3 * s1 * s2 ) - ( c1 * s3 ),
						                    ( c2 * s1 ),
						                    ( c1 * c3 ) + ( s1 * s2 * s3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::XYZ:
				{
					const glm::vec3 row_0 { ( c2 * c3 ), -( c2 * s3 ), s2 };
					const glm::vec3 row_1 { ( c1 * s3 ) + ( c3 * s1 * s2 ),
						                    ( c1 * c3 ) - ( s1 * s2 * s3 ),
						                    -( c2 * s1 ) };
					const glm::vec3 row_2 { ( s1 * s3 ) - ( c1 * c3 * s2 ),
						                    ( c3 * s1 ) + ( c1 * s2 * s3 ),
						                    ( c1 * c2 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::YXZ:
				{
					const glm::vec3 row_0 { ( c1 * c3 ) + ( s1 * s2 * s3 ), ( c3 * s1 * s2 ) - ( c1 * s3 ), c2 * s1 };
					const glm::vec3 row_1 { c2 * s3, c2 * c3, -s2 };
					const glm::vec3 row_2 { ( c1 * s2 * s3 ) - ( c3 * s1 ), ( c1 * c3 * s2 ) + ( s1 * s3 ), c1 * c2 };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::YZX:
				{
					const glm::vec3 row_0 { ( c1 * c2 ),
						                    ( s1 * s3 ) - ( c1 * c3 * s2 ),
						                    ( c3 * s1 ) + ( c1 * s2 * s3 ) };
					const glm::vec3 row_1 { s2, c2 * c3, -( c2 * s3 ) };
					const glm::vec3 row_2 { -( c2 * s1 ),
						                    ( c1 * s3 ) + ( c3 * s1 * s2 ),
						                    ( c1 * c3 ) - ( s1 * s2 * s3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::ZYX: // Roll, Pitch, Yaw
				{
					const glm::vec3 row_0 { ( c1 * c2 ),
						                    ( c1 * s2 * s3 ) - ( c3 * s1 ),
						                    ( s1 * s3 ) + ( c1 * c3 * s2 ) };
					const glm::vec3 row_1 { ( c2 * s1 ),
						                    ( c1 * c3 ) + ( s1 * s2 * s3 ),
						                    ( c3 * s1 * s2 ) - ( c1 * s3 ) };
					const glm::vec3 row_2 { -s2, c2 * s3, c2 * c3 };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			case RotationOrder::ZXY: // Roll, Yaw, Pitch
				{
					const glm::vec3 row_0 { ( c1 * c3 ) - ( s1 * s2 * s3 ),
						                    -( c2 * s1 ),
						                    ( c1 * s3 ) + ( c3 * s1 * s2 ) };

					const glm::vec3 row_1 { ( c3 * s1 ) + ( c1 * s2 * s3 ),
						                    ( c1 * c2 ),
						                    ( s1 * s3 ) - ( c1 * c3 * s2 ) };

					const glm::vec3 row_2 { -( c2 * s3 ), ( s2 ), ( c2 * c3 ) };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return mat;
				}
			default:
				throw std::runtime_error( "Unimplemented rotation order" );
		}
	}

} // namespace fgl::engine
