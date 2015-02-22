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

#ifndef DSPR_CLIENT_H
#define DSPR_CLIENT_H

#include "dspr.h"

#pragma GCC visibility push( default )

struct dspr_client_private_t;
class dspr_client_t
{
    public:

        dspr_client_t();
        ~dspr_client_t();

        void initialize();

        void set_sample_rate( unsigned sample_rate );
        unsigned sample_rate();

        void set_channels( unsigned channels );
        unsigned channels();

        void set_server_ip( std :: string ip );
        std :: string server_ip();

        void set_server_port( unsigned port );
        unsigned server_port();

        void set_server_unix_path( std :: string path );
        std :: string server_unix_path();

        bool activate();
        void deactivate();

        bool prepare_noninterleaved_float( unsigned samples );
        bool prepare_noninterleaved_short( unsigned samples );

        bool write_channel( const float * buffer );
        bool read_channel( float * buffer );
        bool write_channel( const short * buffer );
        bool read_channel( short * buffer );

        bool interleaved_write( const float * buffer, unsigned samples );
        bool interleaved_read( float * buffer );
        bool interleaved_write( const short * buffer, unsigned samples );
        bool interleaved_read( short * buffer );

        bool try_activate();
        bool is_active();

        bool send_message_header( dspr_actions_t action, unsigned length );
        bool send_message_body( const char * body, unsigned size = 0 );

    private:

        bool prepare( unsigned samples, size_t unit_size, dspr_actions_t action );
        dspr_client_private_t * self;
};

#pragma GCC visibility pop

#endif
