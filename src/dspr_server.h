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

#ifndef DSPR_SERVER_H
#define DSPR_SERVER_H

#include "dspr.h"
#include "server.h"
#include "dynamic_buffer.h"
#include "processor.h"

class dspr_client_t : public client_t
{
    public:

    dspr_client_t( socket_t socket ) : client_t( socket )
    {
        action = 0;
        bytes_pending_to_read = 0;
    }

    unsigned action;
    unsigned bytes_pending_to_read;
    dynamic_buffer_t buffer;
    dynamic_buffer_t output_buffer;

    unsigned sample_rate;
    unsigned channels;
};

class dspr_server_t : public server_t
{
    public:

        processor_t processor;

    protected:

        virtual void on_connection();
        virtual void on_input( client_t * client );
        virtual void on_output( client_t * client );
        virtual void on_exception( client_t * client );
        virtual void on_next_iteration();
};

#endif
