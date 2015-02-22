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
#include "socket.h"
#include "sigpipe.h"

struct dspr_client_private_t
{
    unsigned samples;

    std :: string server_ip;
    unsigned server_port;
    std :: string server_unix_path;

    socket_t socket;
    sigpipe_t sigpipe;

    unsigned sample_rate;
    unsigned channels;

    bool already_tried_to_connect;

    dspr_actions_t last_action;
    unsigned last_message_length;
};

dspr_client_t :: dspr_client_t()
{
    self = new dspr_client_private_t;
}

dspr_client_t :: ~dspr_client_t()
{
    delete self;
}

void dspr_client_t :: initialize()
{
    socket_t :: initialize();

    self->samples = 0;

    self->sample_rate = 0;
    self->channels = 0;

    self->already_tried_to_connect = false;

    char * env = getenv( "DSPR" );
    if( env == 0 )
    {
        self->server_ip = "127.0.0.1";
        self->server_port = 6363;
        self->server_unix_path = "/tmp/dspr-1";
    }
    else if( env[ 0 ] == ':' )
    {
        self->server_ip = "127.0.0.1";
        self->server_port = atoi( env + 1 );
        self->server_unix_path = "";
    }
    else
    {
        char * found;

        found = strchr( env, '/' );
        if( found != 0 )
        {
            self->server_ip = "";
            self->server_port = 0;
            self->server_unix_path = env;
        }
        else
        {
            found = strchr( env, ':' );
            if( found == 0 )
            {
                self->server_ip = std :: string( env );
                self->server_port = 6363;
            }
            else
            {
                self->server_ip = std :: string( env, size_t( found - env ) );
                self->server_port = atoi( found + 1 );
            }
            self->server_unix_path = "";
        }
    }
}

void dspr_client_t :: set_sample_rate( unsigned sample_rate )
{
    self->sample_rate = sample_rate;

    if( is_active() == false ) return;
    if( send_message_header( dspr_action_set_sample_rate, sizeof( unsigned ) ) == false ) return;
    send_message_body( ( const char * )&self->sample_rate );
}

unsigned dspr_client_t :: sample_rate()
{
    return self->sample_rate;
}

void dspr_client_t :: set_channels( unsigned channels )
{
    self->channels = channels;
}

unsigned dspr_client_t :: channels()
{
    return self->channels;
}

void dspr_client_t :: set_server_ip( std :: string ip )
{
    self->server_ip = ip;
}

std :: string dspr_client_t :: server_ip()
{
    return self->server_ip;
}

void dspr_client_t :: set_server_port( unsigned port )
{
    self->server_port = port;
}

unsigned dspr_client_t :: server_port()
{
    return self->server_port;
}

void dspr_client_t :: set_server_unix_path( std :: string path )
{
    self->server_unix_path = path;
}

std :: string dspr_client_t :: server_unix_path()
{
    return self->server_unix_path;
}

bool dspr_client_t :: send_message_header( dspr_actions_t action, unsigned length )
{
    unsigned value = action;
    self->last_action = action;
    self->last_message_length = length;

    reset_errno();
    if( self->socket.full_write( ( const char * )&value, sizeof( unsigned ) ) == false )
    {
        self->socket.close();

        set_error( "unable to send message header for action %i", action );
        return false;
    }

    value = length;

    reset_errno();
    if( self->socket.full_write( ( const char * )&value, sizeof( unsigned ) ) == false )
    {
        self->socket.close();

        set_error( "unable to send message length for action %i", action );
        return false;
    }

    return true;
}

bool dspr_client_t :: send_message_body( const char * body, unsigned size )
{
    if( size == 0 ) size = self->last_message_length;

    reset_errno();
    if( self->socket.full_write( body, size ) == false )
    {
        self->socket.close();

        set_error( "unable to send message body for action %i", self->last_action );
        return false;
    }

    return true;
}

bool dspr_client_t :: activate()
{
    if( is_active() ) return true;

    self->already_tried_to_connect = true;
    if( self->server_unix_path.empty() == false )
    {
        if( self->socket.create_unix() == false ) goto tcp_activate;
        if( self->socket.connect_unix( self->server_unix_path.c_str() ) == false )
        {
            self->socket.close();
            goto tcp_activate;
        }

        goto activation_message;
    }

tcp_activate:

    if( self->server_ip.empty() == false )
    {

        if( self->socket.create() == false ) return false;
        self->socket.set_nodelay();

        if( self->socket.connect( self->server_ip.c_str(), self->server_port ) == false ) goto fail;
    }

activation_message:

    self->sigpipe.block();

    if( send_message_header( dspr_action_set_sample_rate, sizeof( unsigned ) ) == false ) goto fail;
    if( send_message_body( ( const char * )&self->sample_rate ) == false ) goto fail;

    if( send_message_header( dspr_action_set_channels, sizeof( unsigned ) ) == false ) goto fail;
    if( send_message_body( ( const char * )&self->channels ) == false ) goto fail;

    self->already_tried_to_connect = false;
    return true;

    fail:
    self->socket.close();

    return false;
}

bool dspr_client_t :: try_activate()
{
    if( is_active() ) return true;

    /* Try every 2 seconds. */
    if( time( 0 ) % 2 != 0 )
    {
        self->already_tried_to_connect = false;
        return false;
    }

    if( self->already_tried_to_connect ) return false;
    return activate();
}

void dspr_client_t :: deactivate()
{
    self->socket.close();

    /* BUG: The whole SIGPIPE thing is just braindead.
            Even if we have executed the syscall when we had SIGPIPE blocked
            the signal sometimes gets through and propagates.
            Introducing artificial sleeps doesn't help either.
    */
    /* self->sigpipe.unblock(); */
}

bool dspr_client_t :: prepare( unsigned samples, size_t unit_size, dspr_actions_t action )
{
    if( is_active() == false ) return false;

    dspr_ensure( self->socket.is_valid() );
    dspr_ensure( samples >= 0 );

    unsigned bytes = samples * unit_size * self->channels;

    self->samples = samples;
    return send_message_header( action, bytes );
}

bool dspr_client_t :: interleaved_write( const float * buffer, unsigned samples )
{
    dspr_ensure( buffer != 0 );
    dspr_ensure( samples > 0 );

    if( is_active() == false ) return false;

    unsigned bytes = samples * sizeof( float ) * self->channels;
    if( prepare( samples, sizeof( float ), dspr_interleaved_float ) == false )
        return false;

    return send_message_body( ( const char * )buffer, bytes );
}

bool dspr_client_t :: interleaved_write( const short * buffer, unsigned samples )
{
    dspr_ensure( buffer != 0 );
    dspr_ensure( samples > 0 );

    if( is_active() == false ) return false;

    unsigned bytes = samples * sizeof( short ) * self->channels;
    if( prepare( samples, sizeof( short ), dspr_interleaved_short_le ) == false )
        return false;

    return send_message_body( ( const char * )buffer, bytes );
}

bool dspr_client_t :: interleaved_read( float * buffer )
{
    dspr_ensure( buffer != 0 );
    dspr_ensure( self->samples > 0 );

    unsigned bytes = self->samples * sizeof( float ) * self->channels;
    if( self->socket.full_read( ( char * )buffer, bytes ) == false )
    {
        set_error( "unable to read a float stream" );
        deactivate();
        return false;
    }

    return true;
}

bool dspr_client_t :: interleaved_read( short * buffer )
{
    dspr_ensure( buffer != 0 );
    dspr_ensure( self->samples > 0 );

    unsigned bytes = self->samples * sizeof( short ) * self->channels;
    if( self->socket.full_read( ( char * )buffer, bytes ) == false )
    {
        set_error( "unable to read a short stream" );
        deactivate();
        return false;
    }

    return true;
}

bool dspr_client_t :: prepare_noninterleaved_float( unsigned samples )
{
    return prepare( samples, sizeof( float ), dspr_noninterleaved_float );
}

bool dspr_client_t :: prepare_noninterleaved_short( unsigned samples )
{
    return prepare( samples, sizeof( short ), dspr_noninterleaved_short_le );
}

bool dspr_client_t :: write_channel( const float * buffer )
{
    dspr_ensure( buffer != 0 );
    return send_message_body( ( const char * )buffer, self->samples * sizeof( float ) );
}

bool dspr_client_t :: write_channel( const short * buffer )
{
    dspr_ensure( buffer != 0 );
    return send_message_body( ( const char * )buffer, self->samples * sizeof( short ) );
}

bool dspr_client_t :: read_channel( float * buffer )
{
    dspr_ensure( buffer != 0 );

    if( self->socket.full_read( ( char * )buffer, self->samples * sizeof( float ) ) == false )
    {
        set_error( "unable to read a channel" );
        deactivate();
        return false;
    }

    return true;
}

bool dspr_client_t :: read_channel( short * buffer )
{
    dspr_ensure( buffer != 0 );

    if( self->socket.full_read( ( char * )buffer, self->samples * sizeof( short ) ) == false )
    {
        set_error( "unable to read a channel" );
        deactivate();
        return false;
    }

    return true;
}

bool dspr_client_t :: is_active()
{
    return self->socket.is_valid();
}
