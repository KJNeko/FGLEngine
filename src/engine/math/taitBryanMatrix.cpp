//
// Created by kj16609 on 2/19/24.
//

#include "taitBryanMatrix.hpp"

#include <stdexcept>
#include <tuple>
#include <utility>

#include "engine/primitives/Rotation.hpp"
#include "engine/primitives/matricies/RotationMatrix.hpp"

namespace fgl::engine
{

	using CosSinPair = std::tuple< float, float >;

	inline std::tuple< CosSinPair, CosSinPair, CosSinPair >
		extract( const glm::vec3 rotation, const RotationOrder order )
	{
		switch ( order )
		{
			case XZY:
				return { { glm::cos( rotation.x ), glm::sin( rotation.x ) },
					     { glm::cos( rotation.z ), glm::sin( rotation.z ) },
					     { glm::cos( rotation.y ), glm::sin( rotation.y ) } };
			case XYZ: // DEFAULT
				return { { glm::cos( rotation.x ), glm::sin( rotation.x ) },
					     { glm::cos( rotation.y ), glm::sin( rotation.y ) },
					     { glm::cos( rotation.z ), glm::sin( rotation.z ) } };
			case YXZ:
				return { { glm::cos( rotation.y ), glm::sin( rotation.y ) },
					     { glm::cos( rotation.x ), glm::sin( rotation.x ) },
					     { glm::cos( rotation.z ), glm::sin( rotation.z ) } };
			case YZX:
				return { { glm::cos( rotation.y ), glm::sin( rotation.y ) },
					     { glm::cos( rotation.z ), glm::sin( rotation.z ) },
					     { glm::cos( rotation.x ), glm::sin( rotation.x ) } };
			case ZYX:
				return { { glm::cos( rotation.z ), glm::sin( rotation.z ) },
					     { glm::cos( rotation.y ), glm::sin( rotation.y ) },
					     { glm::cos( rotation.x ), glm::sin( rotation.x ) } };
			case ZXY:
				return { { glm::cos( rotation.z ), glm::sin( rotation.z ) },
					     { glm::cos( rotation.x ), glm::sin( rotation.x ) },
					     { glm::cos( rotation.y ), glm::sin( rotation.y ) } };
			case END_OF_ENUM:
				throw std::runtime_error( "Unimplemented rotation order" );
		}

		FGL_UNREACHABLE();
	}

	glm::mat3 taitBryanMatrix( const glm::vec3 rotation, const RotationOrder order )
	{
		glm::mat3 mat { 1.0f };

		const auto [ p1, p2, p3 ] = extract( rotation, order );

		const auto& [ c1, s1 ] = p1;
		const auto& [ c2, s2 ] = p2;
		const auto& [ c3, s3 ] = p3;

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
					return RotationMatrix( mat );
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
					return RotationMatrix( mat );
				}
			case RotationOrder::YXZ:
				{
					const glm::vec3 row_0 { ( c1 * c3 ) + ( s1 * s2 * s3 ), ( c3 * s1 * s2 ) - ( c1 * s3 ), c2 * s1 };
					const glm::vec3 row_1 { c2 * s3, c2 * c3, -s2 };
					const glm::vec3 row_2 { ( c1 * s2 * s3 ) - ( c3 * s1 ), ( c1 * c3 * s2 ) + ( s1 * s3 ), c1 * c2 };

					mat[ 0 ] = glm::vec4( row_0, 0.0f );
					mat[ 1 ] = glm::vec4( row_1, 0.0f );
					mat[ 2 ] = glm::vec4( row_2, 0.0f );
					return RotationMatrix( mat );
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
					return RotationMatrix( mat );
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
					return RotationMatrix( mat );
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
					return RotationMatrix( mat );
				}
			case RotationOrder::END_OF_ENUM:
				[[fallthrough]];
			default:
				throw std::runtime_error( "Unimplemented rotation order" );
		}
	}

} // namespace fgl::engine
