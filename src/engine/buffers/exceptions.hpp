//
// Created by kj16609 on 6/26/24.
//

#pragma once

#include <exception>

namespace fgl::engine
{

	struct EngineException : public std::runtime_error
	{
		explicit EngineException( const char* str ) : std::runtime_error( str ) {}
	};

	struct BufferException : public EngineException
	{
		BufferException() = delete;

		explicit BufferException( const char* str ) : EngineException( str ) {}
	};

	struct BufferOOM : public BufferException
	{
		explicit BufferOOM() : BufferException( "Buffer OOM" ) {}
	};

} // namespace fgl::engine
