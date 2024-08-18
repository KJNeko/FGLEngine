//
// Created by kj16609 on 3/1/24.
//

#pragma once

#define FGL_DELETE_DEFAULT_CTOR( ClassName ) ClassName() = delete;
#define FGL_DELETE_COPY_ASSIGN( ClassName ) ClassName& operator=( const ClassName& ) = delete;
#define FGL_DELETE_COPY_CTOR( ClassName ) ClassName( const ClassName& ) = delete;
#define FGL_DELETE_MOVE_ASSIGN( ClassName ) ClassName& operator=( ClassName&& ) = delete;
#define FGL_DELETE_MOVE_CTOR( ClassName ) ClassName( ClassName&& ) = delete;
#define FGL_DELETE_COPY( ClassName ) FGL_DELETE_COPY_CTOR( ClassName ) FGL_DELETE_COPY_ASSIGN( ClassName )
#define FGL_DELETE_MOVE( ClassName ) FGL_DELETE_MOVE_CTOR( ClassName ) FGL_DELETE_MOVE_ASSIGN( ClassName )
#define FGL_DELETE_ALL_Ro5( ClassName )                                                                                \
	FGL_DELETE_DEFAULT_CTOR( ClassName ) FGL_DELETE_COPY( ClassName ) FGL_DELETE_MOVE( ClassName )

#define FGL_FLATTEN [[gnu::flatten]]
#define FGL_ARTIFICIAL [[gnu::artificial]]
#define FGL_HOT [[gnu::hot]]
#define FGL_COLD [[gnu::cold]]
#define FGL_FLATTEN_HOT FGL_FLATTEN FGL_HOT
#define FGL_FORCE_INLINE [[gnu::always_inline]]
#define FGL_FORCE_INLINE_FLATTEN FGL_FLATTEN FGL_FORCE_INLINE

#ifndef NDEBUG
#define FGL_ASSUME( ... )                                                                                              \
	FGL_ASSERT( !( __VA_ARGS__ ), "FGL_ASSUME: Check failed!" );                                                       \
	[[gnu::assume( __VA_ARGS__ )]]
#else
#define FGL_ASSUME( ... ) [[gnu::assume( __VA_ARGS__ )]]
#endif

#define FGL_ALIGN( bytesize ) [[gnu::alligned( bitsize )]]

#define FGL_FUNC_CLEANUP( func ) [[gnu::cleanup( func )]]

//! Warns if the variable is used as a string (strlen)
#define FGL_NONSTRING_DATA [[gnu::nonstring]]

//! Warns if the structure field is not alligned with a set number of bytes
#define FGL_STRICT_ALIGNMENT( bytesize ) [[gnu::warn_if_not_aligned( bytesize )]]

#ifndef NDEBUG
#include <stdexcept>
#define FGL_ASSERT( test, msg )                                                                                        \
	if ( !( test ) ) throw std::runtime_error( msg );
#else
#define FGL_ASSERT( test, msg )
#endif

#ifndef NDEBUG
#include <utility>
#define FGL_UNREACHABLE()                                                                                              \
	FGL_ASSERT( false, "Should have been unreachable!" );                                                              \
	std::unreachable()
#else
#define FGL_UNREACHABLE() std::unreachable()
#endif