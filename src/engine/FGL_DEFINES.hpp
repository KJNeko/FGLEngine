//
// Created by kj16609 on 3/1/24.
//

#pragma once

#ifndef FGL_FORCE_NOTHING

#ifdef __GNUC__
#define FGL_FLATTEN __attribute__( ( flatten ) )
#define FGL_FLATTEN_HOT __attribute__( ( flatten, hot ) )
#define FGL_ARTIFICIAL __attribute__( ( artificial ) )
#define FGL_HOT __attribute__( ( hot ) )
#define FGL_COLD __attribute__( ( cold ) )
#define FGL_FORCE_INLINE __attribute__( ( always_inline ) )
#define FGL_FORCE_INLINE_FLATTEN __attribute__( ( always_inline, flatten ) )
#define FGL_ASSUME( ... ) __attribute__( ( assume( __VA_ARGS__ ) ) )

#define FGL_ALIGN( bytesize ) __attribute__( ( alligned( bitsize ) ) )

#define FGL_FUNC_CLEANUP( func ) __attribute__( ( cleanup( func ) ) )

//! Warns if the variable is used as a string (strlen)
#define FGL_NONSTRING_DATA __attribute__( ( nonstring ) )

//! Warns if the structure field is not alligned with a set number of bytes
#define FGL_STRICT_ALIGNMENT( bytesize ) __attribute__( ( warn_if_not_aligned( bytesize ) ) )

#endif

#else
#define FGL_FLATTEN
#define FGL_FLATTEN_HOT
#define FGL_FORCE_INLINE
#define FGL_FORCE_INLINE_FLATTEN

#endif