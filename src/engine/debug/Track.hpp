//
// Created by kj16609 on 1/22/25.
//
#pragma once
#include <stacktrace>
#include <vector>

#include "utility/TString.hpp"

namespace fgl::engine::debug
{

	using TrackUID = std::size_t;

	struct TrackInfo
	{
		std::size_t size;
		std::stacktrace trace;
		std::string_view group;
		std::string_view name;
	};

	std::size_t
		registerTrack( std::string_view group, std::string_view name, std::size_t size, const std::stacktrace& trace );

	void deregisterTrack( std::size_t UID );

	std::vector< TrackInfo > getTracks( std::string_view group, std::string_view name );
	std::vector< TrackInfo > getAllTracks();

	template < TString Group, TString Name >
	class Track
	{
		TrackUID UID;

	  public:

		explicit Track( std::size_t size = 0, std::stacktrace trace = std::stacktrace::current() ) :
		  UID( registerTrack( Group, Name, size, trace ) )
		{}

		~Track() { deregisterTrack( UID ); }
	};

} // namespace fgl::engine::debug