//
// Created by kj16609 on 1/22/25.
//
#include "Track.hpp"

#include <cassert>
#include <unordered_map>

namespace fgl::engine::debug
{

	inline static TrackUID UIDS { 0 };

	inline static std::unordered_map< TrackUID, TrackInfo > track_info {};

	std::size_t registerTrack( std::string_view group, std::string_view name, std::size_t size, std::stacktrace& trace )
	{
		TrackInfo info {};
		info.size = size;
		info.trace = trace;
		info.group = group;
		info.name = name;

		const auto new_uid { UIDS++ };

		track_info.emplace( new_uid, std::move( info ) );

		return new_uid;
	}

	void deregisterTrack( std::string_view group, std::string_view name, std::size_t UID )
	{
		assert( track_info.erase( UID ) > 0 && "Unable to find UID" );
	}

	std::vector< TrackInfo > getTracks( const std::string_view group, const std::string_view name )
	{
		std::vector< TrackInfo > tracks {};
		tracks.reserve( track_info.size() );

		for ( const auto& [ uid, info ] : track_info )
		{
			if ( info.group == group && info.name == name ) tracks.emplace_back( info );
		}

		return tracks;
	}

	std::vector< TrackInfo > getAllTracks()
	{
		std::vector< TrackInfo > tracks {};

		tracks.reserve( track_info.size() );
		for ( const auto& [ uid, info ] : track_info )
		{
			tracks.emplace_back( info );
		}

		return tracks;
	}

} // namespace fgl::engine::debug