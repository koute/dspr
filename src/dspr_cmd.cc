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

void print_syntax()
{
    fprintf( stderr, ""
    "Usage: dspr-cmd [flags] <action>\n"
    "\n"
    "Accepted actions:\n"
    "  quit            Forces the server to quit.\n"
    "  enable          Enables processing. (default)\n"
    "  disable         Disables processing.\n"
    "  toggle          Toggles processing.\n"
    );
    exit( 1 );
}

int main( int argc, char * argv[] )
{
    if( argc != 2 ) print_syntax();

    dspr_actions_t action;
    if( !strcmp( argv[ 1 ], "quit" ) ) action = dspr_action_quit;
    else if( !strcmp( argv[ 1 ], "enable" ) ) action = dspr_action_enable;
    else if( !strcmp( argv[ 1 ], "disable" ) ) action = dspr_action_disable;
    else if( !strcmp( argv[ 1 ], "toggle" ) ) action = dspr_action_toggle;
    else print_syntax();

    dspr_client_t client;
    client.initialize();
    if( client.activate() == false ) return 1;
    client.send_message_header( action, 0 );
    client.deactivate();
    
    return 0;
}
