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

#ifndef SOCKET_H
#define SOCKET_H

#include "dspr.h"

class socket_t
{
    public:

        static bool initialize();
        static void destroy();

        socket_t();
        socket_t( int socket, bool is_unix = false );

        bool create();
        bool create_unix();
        void close();

        bool set_nodelay();

        bool connect( const char * ip, int port );
        bool connect_unix( const char * path );
        bool bind( unsigned port );
        bool bind_unix( const char * path );
        socket_t accept();

        bool getip( std :: string& ip );

        int write( const char * buffer, int length );
        int read( char * buffer, int length );

        bool full_write( const char * buffer, int length );
        bool full_read( char * buffer, int length );

        bool is_valid();

        int raw_socket;
        bool is_unix;
};

#endif
