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

#include "dspr_server.h"

void dspr_server_t :: on_connection()
{
    socket_t socket = accept();
    if( socket.is_valid() == false )
        return;

    socket.set_nodelay();

    dspr_client_t * client = new dspr_client_t( socket );
    client->bytes_pending_to_read = sizeof( unsigned ) * 2;
    clients.push_back( client );

    std :: string ip;
    socket.getip( ip );
    printf( "> (%i) New connection from <%s>.\n", socket.raw_socket, ip.c_str() );
}

void dspr_server_t :: on_input( client_t * base_client )
{
    dspr_client_t * client = ( dspr_client_t * )base_client;
    client->buffer.resize_to_at_least( client->bytes_pending_to_read + client->buffer.absolute_length() );

    int length = client->socket.read( client->buffer.buffer_end(), client->bytes_pending_to_read );
    if( length == 0 )
    {
        client->is_valid = false;
        return;
    }
    client->buffer.register_manual_append( length );
    client->bytes_pending_to_read -= length;

    if( client->bytes_pending_to_read != 0 ) return;

    again:

    if( client->action == 0 )
    {
        client->action = client->buffer.read_unsigned();
        client->bytes_pending_to_read = client->buffer.read_unsigned();

        if( client->action == 0 )
        {
            client->is_valid = false;
            return;
        }

        if( client->bytes_pending_to_read == 0 ) goto again;
        else return;
    }

    if( client->action == dspr_action_quit )
        running = false;
    else if( client->action == dspr_action_set_sample_rate )    client->sample_rate = client->buffer.read_unsigned();
    else if( client->action == dspr_action_set_channels )       client->channels = client->buffer.read_unsigned();
    else if( client->action == dspr_interleaved_float )         processor.process( client->buffer.buffer(), client->buffer.length(), format_float, format_endianless, format_interleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_interleaved_short_le )      processor.process( client->buffer.buffer(), client->buffer.length(), format_short, format_le, format_interleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_interleaved_short_be )      processor.process( client->buffer.buffer(), client->buffer.length(), format_short, format_be, format_interleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_noninterleaved_float )      processor.process( client->buffer.buffer(), client->buffer.length(), format_float, format_endianless, format_noninterleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_noninterleaved_short_le )   processor.process( client->buffer.buffer(), client->buffer.length(), format_short, format_le, format_noninterleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_noninterleaved_short_be )   processor.process( client->buffer.buffer(), client->buffer.length(), format_short, format_be, format_noninterleaved, client->channels, client->sample_rate );
    else if( client->action == dspr_action_disable )            processor.enabled = false;
    else if( client->action == dspr_action_enable )             processor.enabled = true;
    else if( client->action == dspr_action_toggle )             processor.enabled = !processor.enabled;

    if( processor.skipped == true )
    {
        client->output_buffer.append( client->buffer.buffer(), client->buffer.length() );
        processor.skipped = false;
    }
    else if( processor.pending == true )
    {
        client->output_buffer.append( processor.buffer(), processor.buffer_length() );
        processor.pending = false;
    }

    client->action = 0;
    client->bytes_pending_to_read = sizeof( unsigned ) * 2;
    client->buffer.clear();
}

void dspr_server_t :: on_output( client_t * base_client )
{
    dspr_client_t * client = ( dspr_client_t * )base_client;

    int wrote = client->socket.write( client->output_buffer.buffer(), client->output_buffer.length() );
    if( wrote <= 0 )
    {
        client->is_valid = false;
        return;
    }

    client->output_buffer.move( wrote );

    if( client->output_buffer.length() == 0 )
        client->output_buffer.clear();
}

void dspr_server_t :: on_exception( client_t * client )
{
    client->is_valid = false;
}

void dspr_server_t :: on_next_iteration()
{
    for( std :: list< client_t * > :: iterator i = clients.begin(); i != clients.end(); ++i )
    {
        if( (*i)->is_valid == true ) continue;

        printf( "> (%i) Disconnecting.\n", (*i)->socket.raw_socket );

        (*i)->socket.close();
        clients.remove( (*i) );
        delete (*i);

        i = clients.begin();
    }

    for( std :: list< client_t * > :: iterator i = clients.begin(); i != clients.end(); ++i )
    {
        dspr_client_t * client = ( dspr_client_t * )(*i);
        client->is_output_ready = client->output_buffer.empty() == false;
    }
}
