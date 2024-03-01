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
#endif

#else
#define FGL_FLATTEN
#define FGL_FLATTEN_HOT
#define FGL_FORCE_INLINE
#define FGL_FORCE_INLINE_FLATTEN

#endif