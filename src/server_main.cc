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

#include "dspr.h"
#include "dspr_server.h"
#include "sigpipe.h"

#include "module_null.h"
#include "module_bs2b.h"

#ifdef WIN32
    #include "module_winamp.h"
    #include "module_vst.h"
#endif

#if defined( __WINE__ ) || !defined( WIN32 )
    #include <sys/stat.h>
    #include <unistd.h>
#endif

#define OPTION( opt ) !strcmp( argv[ i ], opt )

static void print_syntax()
{
    fprintf( stderr, ""
        "Usage: dspr [server flags] [module] [module flags] [another_module] (...)\n"
        "\n"
        "Flags accepted by the server:\n"
        "  -p <port>            Sets port on which server will listen. (default: 6363)\n"
        "  -d                   Won't quit when no plugins will be set.\n"
        "  -u                   Use an UNIX socket. (Default on Linux and WINE builds.)\n"
        "  -t                   Use a TCP socket. (Default on MinGW builds.)\n"
        "  -r <path>            Sets UNIX socket path. (default: /tmp/dspr-1)\n"
        "  -a                   Tries to aquire realtime scheduling. (Requires setuid root; ignored on MinGW builds.)\n"
        "  -s <uid>             Set UID to <uid>. (Useful in conjunction with -a; ignored on MinGW builds.)\n"
        "\n"
        "Availible modules:\n"
    );

    for( std :: vector< meta_module_t * > :: iterator i = processor_t :: registered_modules.begin(); i != processor_t :: registered_modules.end(); ++i )
    {
        printf( "  %s -- %s (%s)\n", (*i)->name(), (*i)->pretty_name(), (*i)->version() );

        const char ** fields = (*i)->fields();
        unsigned max_length = 0;

        for( unsigned n = 0; fields[ n ] != 0; ++n )
        {
            max_length = std :: max( max_length, unsigned( strlen( fields[ n ] ) ) );
        }

        for( unsigned n = 0; fields[ n ] != 0; ++n )
        {
            field_type type = (*i)->get_type( fields[ n ] );
            std :: string readable_type;

            switch( type )
            {
                case field_string:
                    readable_type = "string";
                    break;

                case field_long:
                    readable_type = "long";
                    break;

                case field_double:
                    readable_type = "double";
                    break;

                case field_bool:
                    readable_type = "bool";
                    break;

                case field_executable:
                    readable_type = "executable";
                    break;

                default:
                    readable_type = "unknown";
            }
            if( (*i)->is_readonly( fields[ n ] ) )
                readable_type += ", readonly";

            printf( "    %s", fields[ n ] );
            for( unsigned i = strlen( fields[ n ] ); i < max_length; ++i )
                printf( " " );
            printf( " (%s)\n", readable_type.c_str() );
        }

    }

    exit( 1 );
}

dspr_server_t server;

void on_quit_signal( int )
{
    server.running = false;
}

int main( int argc, char * argv[] )
{
    socket_t :: initialize();

    processor_t :: register_module( &g_meta_module_null );
    processor_t :: register_module( &g_meta_module_bs2b );
    #ifdef WIN32
        processor_t :: register_module( &g_meta_module_winamp );
        processor_t :: register_module( &g_meta_module_vst );
    #endif

    signal( SIGINT, on_quit_signal );

    int port = 6363;
    bool dummy = false;
    #if defined( WIN32 ) && !defined( __WINE__ )
        bool use_unix_socket = false;
    #else
        bool use_unix_socket = true;
    #endif
    std :: string unix_socket_path = "/tmp/dspr-1";
    bool realtime = false;
    int uid = 0;
    
    #if !defined( WIN32 ) || defined( __WINE__ )
        uid = getuid();
    #endif

    for( int i = 1; i < argc; i++ )
    {
        if( OPTION( "-p" ) || OPTION( "--port" ) )
        {
            i++;
            if( i >= argc ) break;

            port = atoi( argv[ i ] );
            continue;
        }
        else if( OPTION( "-d" ) || OPTION( "--dummy" ) )
        {
            dummy = true;
            continue;
        }
        else if( OPTION( "-h" ) || OPTION( "--help" ) )
        {
            print_syntax();
            return 0;
        }
        else if( OPTION( "-u" ) || OPTION( "--use-unix-socket" ) )
        {
            use_unix_socket = true;
            continue;
        }
        else if( OPTION( "-t" ) || OPTION( "--use-tcp-socket" ) )
        {
            use_unix_socket = false;
            continue;
        }
        else if( OPTION( "-r" ) || OPTION( "--unix-socket-path" ) )
        {
            i++;
            if( i >= argc ) break;

            unix_socket_path = argv[ i ];
            continue;
        }
        else if( OPTION( "-a" ) || OPTION( "--realtime" ) )
        {
            realtime = true;
            continue;
        }
        else if( OPTION( "-s" ) || OPTION( "--set-uid" ) )
        {
            i++;
            if( i >= argc ) break;

            uid = atoi( argv[ i ] );
            continue;
        }

        if( argv[ i ][ 0 ] == '-' && argv[ i ][ 1 ] == '-' )
        {
            module_t * last_module = server.processor.modules.back();
            if( last_module == 0 ) print_syntax();

            const char * key = argv[ i ] + 2;

            meta_module_t * meta_module = last_module->get_meta_module();
            if( meta_module->has_field( key ) == false )
            {
                reset_errno();
                set_error( "field '%s' doesn't exist for plugin '%s'", key, meta_module->name() );
                return 1;
            }

            if( meta_module->is_executable( key ) == true )
            {
                if( last_module->execute_field( key ) == false )
                {
                    reset_errno();
                    set_error( "unable to execute field '%s' for plugin '%s'", key, meta_module->name() );
                    return 1;
                }
                continue;
            }

            if( meta_module->is_readonly( key ) == true )
            {
                reset_errno();
                set_error( "field '%s' is readonly for plugin '%s'", key, meta_module->name() );
                return 1;
            }

            i++;
            if( i >= argc ) break;
            const char * value = argv[ i ];

            if( last_module->set_field( key, value ) == false )
            {
                reset_errno();
                set_error( "unable to set field '%s' for plugin '%s'", key, meta_module->name() );
                return 1;
            }

            continue;
        }

        if( server.processor.add_module( argv[ i ] ) == false )
        {
            reset_errno();
            set_error( "unable to load module '%s'", argv[ i ] );
            return 1;
        }
        else
            printf( "Loaded module '%s'.\n", argv[ i ] );
    }

    if( dummy == false && server.processor.modules.empty() )
    {
        puts( "No modules specified; quitting." );
        return 1;
    }

    #if !defined( WIN32 ) || defined( __WINE__ )
        if( realtime == true )
        {
            sched_param param;
            param.sched_priority = sched_get_priority_max( SCHED_RR );
            if( sched_setscheduler( getpid(), SCHED_RR, &param ) != 0 )
                perror( "Error: unable to set scheduling policy to SCHED_RR" );
            else
                puts( "Scheduling policy set to SCHED_RR." );
        }
        setuid( uid );
    #endif

    if( use_unix_socket == true )
    {
        #if !defined( WIN32 ) || defined( __WINE__ )
            if( server.create_unix() == false ) return 1;
            if( server.bind_unix( unix_socket_path.c_str() ) == false ) return 2;
            chmod( unix_socket_path.c_str(), 0777 );

            printf( "Ready to process on unix socket: '%s'.\n", unix_socket_path.c_str() );
        #else
            set_error( "UNIX sockets are unsupported" );
        #endif
    }
    else
    {
        if( server.create() == false ) return 1;
        if( server.bind( port ) == false ) return 2;

        printf( "Ready to process on port %i.\n", port );
    }

    sigpipe_t sigpipe;
    sigpipe.block();
    if( server.run() == false ) return 3;
    server.disconnect_all_clients();

    return 0;
}
