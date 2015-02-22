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

#include "dspr_client.h"
#include <jack/jack.h>

pthread_mutex_t g_mutex;
pthread_cond_t g_condition = PTHREAD_COND_INITIALIZER;

jack_port_t * g_input_port[ 2 ];
jack_port_t * g_output_port[ 2 ];
dspr_client_t g_filter;

jack_port_t * g_physical_port[ 2 ];
jack_client_t * g_jack_client;

bool g_running = true;

struct reconnection_t
{
    std :: string original_from;
    std :: string original_to;
    std :: string to;
};

std :: vector< reconnection_t > g_reconnect_queue;

int on_process( jack_nframes_t nframes, void * arg )
{
    if( nframes == 0 )
        return 0;

    jack_default_audio_sample_t * output[ 2 ];
    jack_default_audio_sample_t * input[ 2 ];
    for( unsigned i = 0; i < 2; ++i )
    {
        output[ i ] = ( jack_default_audio_sample_t * )jack_port_get_buffer( g_output_port[ i ], nframes );
        input[ i ] = ( jack_default_audio_sample_t * )jack_port_get_buffer( g_input_port[ i ], nframes );
    }

    if( g_filter.is_active() == false && g_filter.try_activate() == false ) goto do_copy;
    if( g_filter.prepare_noninterleaved_float( nframes ) == false ) goto failed;

    for( unsigned i = 0; i < 2; ++i )
        if( g_filter.write_channel( input[ i ] ) == false ) goto failed;

    for( unsigned i = 0; i < 2; ++i )
        if( g_filter.read_channel( output[ i ] ) == false ) goto failed;


    return 0;

    failed:

    do_copy:

        for( unsigned i = 0; i < 2; ++i )
            memcpy( output[ i ], input[ i ], sizeof( jack_default_audio_sample_t ) * nframes );

    return 0;
}

int on_sample_rate_change( jack_nframes_t nframes, void * arg )
{
    printf( "Sample rate is now %lu\n", ( long unsigned int )nframes );
    return 0;
}

void on_error( const char * message )
{
    fprintf( stderr, "JACK error: %s\n", message );
}

void on_sigint( int signum )
{
    g_running = false;
    pthread_mutex_lock( &g_mutex );
    pthread_cond_broadcast( &g_condition );
    pthread_mutex_unlock( &g_mutex );
}

void on_shutdown( void * arg )
{
    on_sigint( SIGINT );
}

void on_port_connected( jack_port_id_t a_id, jack_port_id_t b_id, int connected, void * arg )
{
    if( connected == 0 || g_running == false ) return;

    jack_port_t * port_a = jack_port_by_id( g_jack_client, a_id );
    jack_port_t * port_b = jack_port_by_id( g_jack_client, b_id );

    if( jack_port_is_mine( g_jack_client, port_a ) != 0 || jack_port_is_mine( g_jack_client, port_b ) != 0 ) return;

    const char * port_a_name = jack_port_name( port_a );
    const char * port_b_name = jack_port_name( port_b );

    pthread_mutex_lock( &g_mutex );
    for( int i = 0; i < 2; ++i )
    {
        if( port_b == g_physical_port[ i ] )
        {
            reconnection_t reconnection;
            reconnection.original_from = port_a_name;
            reconnection.original_to   = port_b_name;
            reconnection.to = jack_port_name( g_input_port[ i ] );
            g_reconnect_queue.push_back( reconnection );
        }
    }
    pthread_cond_broadcast( &g_condition );
    pthread_mutex_unlock( &g_mutex );
}

int main( int argc, char * argv[] )
{
    sched_param param;
    param.sched_priority = sched_get_priority_max( SCHED_RR );
    if( sched_setscheduler( getpid(), SCHED_RR, &param ) != 0 )
        perror( "Error: unable to set scheduling policy to SCHED_RR" );

    setuid( getuid() );
    setgid( getgid() );
    signal( SIGINT, on_sigint );

    pthread_mutex_init( &g_mutex, 0 );
    jack_set_error_function( on_error );

    if( (g_jack_client = jack_client_new( "dspr" )) == 0 )
    {
        fprintf( stderr, "Error: jack server not running?\n" );
        return 1;
    }

    jack_set_process_callback( g_jack_client, on_process, 0 );
    jack_set_sample_rate_callback( g_jack_client, on_sample_rate_change, 0 );
    jack_on_shutdown( g_jack_client, on_shutdown, 0 );

    g_filter.initialize();
    g_filter.set_sample_rate( jack_get_sample_rate( g_jack_client ) );
    g_filter.set_channels( 2 );
    g_filter.activate();

    for( unsigned i = 0; i < 2; ++i )
    {
        char input_name[] = "input1";
        char output_name[] = "output1";
        input_name[ 5 ] += i;
        output_name[ 6 ] += i;

        g_input_port[ i ] = jack_port_register( g_jack_client, input_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0 );
        g_output_port[ i ] = jack_port_register( g_jack_client, output_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
    }

    if( jack_set_port_connect_callback( g_jack_client, on_port_connected, 0 ) != 0 )
    {
        fprintf( stderr, "Error: jack_set_port_connect_callback failed\n" );
        return 1;
    }

    puts( "Activating..." );
    if( jack_activate( g_jack_client ) )
    {
        fprintf( stderr, "Error: cannot activate client." );
        return 1;
    }

    const char ** input_ports;
    if( (input_ports = jack_get_ports( g_jack_client, 0, 0, JackPortIsPhysical|JackPortIsInput )) == 0 )
    {
        fprintf( stderr, "Error: cannot find any physical input ports\n" );
        return 1;
    }

    g_physical_port[ 0 ] = jack_port_by_name( g_jack_client, input_ports[ 0 ] );
    g_physical_port[ 1 ] = jack_port_by_name( g_jack_client, input_ports[ 1 ] );

    for( unsigned i = 0; i < 2; ++i )
    {
        const char ** connections = jack_port_get_all_connections( g_jack_client, g_physical_port[ i ] );
        if( connections != 0 )
        {
            pthread_mutex_lock( &g_mutex );

            for( unsigned n = 0; connections[ n ] != 0; ++n )
            {
                const char * connection = connections[ n ];

                reconnection_t reconnection;
                reconnection.original_from = connection;
                reconnection.original_to   = input_ports[ i ];
                reconnection.to = jack_port_name( g_input_port[ i ] );
                g_reconnect_queue.push_back( reconnection );
            }

            pthread_mutex_unlock( &g_mutex );
            jack_free( connections );
        }
    }

    for( unsigned i = 0; i < 2; ++i )
    {
        if( jack_connect( g_jack_client, jack_port_name( g_output_port[ i ] ), input_ports[ i ] ) )
        {
            fprintf( stderr, "Error: cannot connect to input port %i.\n", i );
            return 1;
        }
    }

    while( g_running == true )
    {
        pthread_mutex_lock( &g_mutex );
        while( g_reconnect_queue.empty() )
        {
            if( g_running == false ) break;
            pthread_cond_wait( &g_condition, &g_mutex );
        }

        if( g_running == false ) break;

        std :: vector< reconnection_t > reconnect_queue_copy = g_reconnect_queue;
        g_reconnect_queue.clear();
        pthread_mutex_unlock( &g_mutex );

        for( unsigned i = 0; i < reconnect_queue_copy.size(); ++i )
        {
            reconnection_t& reconnection = reconnect_queue_copy[ i ];
            printf( "Reconnecting %s->%s to %s...\n", reconnection.original_from.c_str(), reconnection.original_to.c_str(), reconnection.to.c_str() );
            jack_disconnect( g_jack_client, reconnection.original_from.c_str(), reconnection.original_to.c_str() );
            jack_connect( g_jack_client, reconnection.original_from.c_str(), reconnection.to.c_str() );

        }
    }

    for( unsigned i = 0; i < 2; ++i )
    {
        const char ** connections = jack_port_get_all_connections( g_jack_client, g_input_port[ i ] );
        if( connections != 0 )
        {
            for( unsigned n = 0; connections[ n ] != 0; ++n )
            {
                const char * connection = connections[ n ];

                jack_disconnect( g_jack_client, connection, jack_port_name( g_input_port[ i ] ) );
                jack_connect( g_jack_client, connection, input_ports[ i ] );
            }

            jack_free( connections );
        }
    }

    jack_deactivate( g_jack_client );
    jack_client_close( g_jack_client );
    g_reconnect_queue.clear();

    return 0;
}
