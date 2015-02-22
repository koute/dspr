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

#include "server.h"

#ifdef USE_NATIVE_WINDOWS_SOCKETS
    #include <winsock2.h>
#endif

server_t :: server_t()
{
    running = true;
}

bool server_t :: run()
{
    fd_set in_set;
    fd_set out_set;
    fd_set exc_set;

    while( running )
    {
        int max_socket = raw_socket;
        struct timeval timeout = { 3, 0 };

        FD_ZERO( &in_set );
        FD_ZERO( &out_set );
        FD_ZERO( &exc_set );
        FD_SET( unsigned( raw_socket ), &in_set );

        bool select_output_too = false;
        for( std :: list< client_t * > :: iterator i = clients.begin(); i != clients.end(); ++i )
        {
            if( (*i)->is_valid == false ) continue;
            if( (*i)->is_output_ready == true )
            {
                select_output_too = true;
                FD_SET( unsigned( (*i)->socket.raw_socket ), &out_set );
            }

            max_socket = std :: max< int >( (*i)->socket.raw_socket, max_socket );
            FD_SET( unsigned( (*i)->socket.raw_socket ), &in_set  );
            FD_SET( unsigned( (*i)->socket.raw_socket ), &exc_set );
        }

        reset_errno();

retry_select:

        if( select_output_too == false )
        {
            if( select( max_socket + 1, &in_set, 0, &exc_set, &timeout ) < 0 )
            {
                if( errno == EINTR ) goto retry_select;

                set_error( "select_failed" );
                return false;
            }
        }
        else
        {
            if( select( max_socket + 1, &in_set, &out_set, &exc_set, &timeout ) < 0 )
            {
                if( errno == EINTR ) goto retry_select;

                set_error( "select_failed" );
                return false;
            }
        }

        if( FD_ISSET( raw_socket, &in_set ) )
            on_connection();

        for( std :: list< client_t * > :: iterator i = clients.begin(); i != clients.end(); ++i )
        {
            client_t * peer = *i;
            if( peer->is_valid == false ) continue;

            if( FD_ISSET( peer->socket.raw_socket, &exc_set ) )
            {
                on_exception( peer );
                continue;
            }

            if( FD_ISSET( peer->socket.raw_socket, &in_set ) )
            {
                on_input( peer );

                if( running == false ) break;
                if( peer->is_valid == false ) continue;
            }

            if( FD_ISSET( peer->socket.raw_socket, &out_set ) )
            {
                on_output( peer );
            }
        }

        on_next_iteration();
    }

    return true;
}

void server_t :: disconnect_all_clients()
{
    for( std :: list< client_t * > :: iterator i = clients.begin(); i != clients.end(); ++i )
        (*i)->socket.close();

    clients.clear();
}
