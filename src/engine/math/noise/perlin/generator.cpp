//
// Created by kj16609 on 3/27/24.
//

#include "generator.hpp"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <glm/geometric.hpp>
#pragma GCC diagnostic pop


#include <tracy/Tracy.hpp>

#include <algorithm>
#include <random>

namespace fgl::engine
{
	//TODO: Figure out how this magic works.
	glm::vec2 randomGradient( int ix, int iy )
	{
		constexpr unsigned w { 8 * sizeof( unsigned ) };
		constexpr unsigned s { w / 2 };
		unsigned a { static_cast< unsigned >( ix ) };
		unsigned b { static_cast< unsigned >( iy ) };

		a *= 3284157443;

		b ^= a << s | a >> ( w - s );
		b *= 1911520717;

		a ^= b << s | b >> ( w - s );
		a *= 2048419325;
		const float val { static_cast< float >( a ) * ( std::numbers::pi_v< float > / ~( ~0u >> 1 ) ) };

		return glm::vec2( sin( val ), cos( val ) );
	}

	float
		dotGridGradiant( const glm::vec< 2, int > i_coord, const glm::vec2 coord, [[maybe_unused]] std::mt19937_64& mt )
	{
		const glm::vec2 gradiant { randomGradient( i_coord.x, i_coord.y ) };

		const glm::vec2 d { coord - static_cast< glm::vec2 >( i_coord ) };

		return glm::dot( d, gradiant );
	}

	float interpolate( const float a0, const float a1, const float w )
	{
		return ( a1 - a0 ) * ( 3.0f - w * 2.0f ) * w * w + a0;
	}

	float perlin( const glm::vec2 coord, std::mt19937_64& mt )
	{
		const glm::vec< 2, int > c0 { glm::round( coord - glm::vec2( 0.5f ) ) };
		const glm::vec< 2, int > c1 { c0 + glm::vec< 2, int >( 1 ) };

		const float x_weight { coord.x - static_cast< float >( c0.x ) };
		const float y_weight { coord.y - static_cast< float >( c0.y ) };

		const float n0 { dotGridGradiant( c0, coord, mt ) };
		const float n1 { dotGridGradiant( { c1.x, c0.y }, coord, mt ) };
		const float ix0 { interpolate( n0, n1, x_weight ) };

		const float n2 { dotGridGradiant( { c0.x, c1.y }, coord, mt ) };
		const float n3 { dotGridGradiant( c1, coord, mt ) };
		const float ix1 { interpolate( n2, n3, x_weight ) };

		return interpolate( ix0, ix1, y_weight );
	}

	std::vector< std::byte >
		generatePerlinImage( const glm::vec< 2, std::size_t > size, const int octives, const std::size_t seed )
	{
		ZoneScoped;
		constexpr std::size_t channel_count { 4 }; // RGBA
		std::vector< std::byte > data { size.x * size.y * channel_count };

		std::mt19937_64 mt { seed };

		for ( std::size_t x = 0; x < size.x; ++x )
		{
			for ( std::size_t y = 0; y < size.y; ++y )
			{
				const auto index { ( y * size.x + x ) * 4 };

				float val { 0.0f };

				float freq { 1 };
				float amp { 1 };

				constexpr float GRID_SIZE { 400.0f };

				for ( int i = 0; i < octives; ++i )
				{
					val += perlin(
							   { ( static_cast< float >( x ) * freq ) / GRID_SIZE,
					             ( static_cast< float >( y ) * freq ) / GRID_SIZE },
							   mt )
					     * amp;

					freq *= 2.0f;
					amp /= 2.0f;
				}

				val *= 1.2f;

				val = std::clamp( val, -1.0f, 1.0f );

				const std::byte color { static_cast< std::byte >( ( val + 1.0f ) * 0.5f * 255.0f ) };

				data[ index ] = color;
				data[ index + 1 ] = color;
				data[ index + 2 ] = color;
				data[ index + 3 ] = std::byte( 255 );
			}
		}

		return data;
	}

} // namespace fgl::engine
