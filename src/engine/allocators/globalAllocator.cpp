//
// Created by kj16609 on 3/11/24.
//

#include <tracy/Tracy.hpp>

#include <cstddef>
#include <cstdlib>
#if TRACY_ENABLE

void* operator new( std::size_t count )
{
	auto ptr = malloc( count );
	TracyAlloc( ptr, count );
	return ptr;
}

void operator delete( void* ptr ) noexcept
{
	TracyFree( ptr );
	free( ptr );
}

void operator delete( void* ptr, [[maybe_unused]] std::size_t size ) noexcept
{
	TracyFree( ptr );
	free( ptr );
}

#endif
