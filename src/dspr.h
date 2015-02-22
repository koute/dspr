/*
    Copyright 2010 (C) Jan Bujak <j@exia.io>

    This file is part of DSPR.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DSPR_H
#define DSPR_H

#ifndef _GNU_SOURCE
    #define _GNU_SOURCE
#endif

#if defined( WIN32 ) || defined( WINDOWS )

    #define VC_EXTRALEAN
    #define WIN32_LEAN_AND_MEAN
    #define _WIN32_WINNT 0x0500

    #include <windows.h>

    #ifndef WINDOWS
        #define WINDOWS
    #endif

    #ifndef __WINE__
        #define USE_NATIVE_WINDOWS_SOCKETS
    #else
        #include <unistd.h>
    #endif

#else

    #include <dlfcn.h>
    #include <unistd.h>

#endif

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>

#include <string>
#include <sstream>
#include <list>
#include <vector>

inline void reset_errno()
{
    errno = 0;
}

inline void set_error( const char * format, ... )
{
    va_list args;
    va_start( args, format );

    char buffer[ 512 ];
    vsnprintf( ( char * )&buffer, 512, format, args );
    va_end( args );

    if( errno != 0 )
    {
        strncat( ( char * )&buffer, ": ", 512 );
        strncat( ( char * )&buffer, strerror( errno ), 512 );
    }

    fprintf( stderr, "error: %s\n", buffer );
}

inline void dspr_ensure_f( bool run, const char * assertion )
{
    if( run == true ) return;
    fprintf( stderr, "assertion failed: %s\n", assertion );
    exit( 1 );
}

#define dspr_ensure( assertion ) dspr_ensure_f( assertion, #assertion )

enum dspr_actions_t
{
    dspr_action_null = 0,
    dspr_action_quit,
    dspr_action_set_sample_rate,
    dspr_action_set_channels,
    dspr_interleaved_float,
    dspr_interleaved_short_le,
    dspr_interleaved_short_be,
    dspr_noninterleaved_float,
    dspr_noninterleaved_short_le,
    dspr_noninterleaved_short_be,
    dspr_action_disable,
    dspr_action_enable,
    dspr_action_toggle
};

#endif
