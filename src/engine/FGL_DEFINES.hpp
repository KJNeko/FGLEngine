//
// Created by kj16609 on 3/1/24.
//

#pragma once

#ifdef __GNUC__
#define FGL_FLATTEN __attribute__( ( flatten ) )
#define FGL_FLATTEN_HOT __attribute__( ( flatten, hot ) )
#define FGL_ARTIFICIAL __attribute__( ( artificial ) )
#define FGL_HOT __attribute__( ( hot ) )
#define FGL_COLD __attribute__( ( cold ) )
#endif