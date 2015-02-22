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

#include <unistd.h>
#include <sys/stat.h>
#include <sched.h>
#include <stdio.h>
#include <vector>

int main( int argc, char * argv[] )
{
    sched_param param;
    param.sched_priority = sched_get_priority_max( SCHED_RR );
    if( sched_setscheduler( getpid(), SCHED_RR, &param ) != 0 )
        perror( "Error: unable to set scheduling policy to SCHED_RR" );
    else
        puts( "Scheduling policy set to SCHED_RR." );

    setuid( getuid() );
    setgid( getgid() );

    static char executable[] = "dspr-server-wine.exe";

    std :: vector< char * > args;
    args.push_back( ( char * )executable );
    for( int i = 1; i < argc; ++i )
        args.push_back( argv[ i ] );

    args.push_back( 0 );

    execvp( executable, &args[ 0 ] );
}
