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

#ifndef SERVER_H
#define SERVER_H

#include "dspr.h"
#include "socket.h"

class client_t
{
    public:

        client_t( socket_t socket )
        {
            this->socket = socket;

            is_output_ready = true;
            is_valid = true;
        }

        bool is_output_ready;
        bool is_valid;
        socket_t socket;
};

class server_t : public socket_t
{
    public:

        server_t();

        bool run();
        bool running;
        void disconnect_all_clients();

    protected:

        virtual void on_connection() = 0;
        virtual void on_input( client_t * client ) = 0;
        virtual void on_output( client_t * client ) = 0;
        virtual void on_exception( client_t * client ) = 0;
        virtual void on_next_iteration() = 0;

        std :: list< client_t * > clients;
};

#endif
