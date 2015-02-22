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

#include "socket.h"

#ifdef USE_NATIVE_WINDOWS_SOCKETS
    #include <winsock2.h>

    /* MinGW doesn't have this in any headers. */

    typedef unsigned short int sa_family_t;
    struct sockaddr_un
    {
        sa_family_t sun_family;
        char sun_path[ 108 ];
    };

#else
    #include <netinet/tcp.h>
    #include <sys/socket.h>
    #include <sys/un.h>
    #include <arpa/inet.h>
#endif

#if defined( PF_INET )
    #define SOCK_AF PF_INET
#else
    #define SOCK_AF AF_INET
#endif

#ifdef USE_NATIVE_WINDOWS_SOCKETS
    typedef int socklen_t;
#endif

bool socket_t :: initialize()
{
    #ifdef USE_NATIVE_WINDOWS_SOCKETS

        WORD version_requested = MAKEWORD( 1, 1 );
        WSADATA wsa_data;
        if( WSAStartup( version_requested, &wsa_data ) != 0 )
        {
            set_error( "WSAStartup failed" );
            return false;
        }

    #endif

    return true;
}

void socket_t :: destroy()
{
    #ifdef USE_NATIVE_WINDOWS_SOCKETS
        WSACleanup();
    #endif
}

socket_t :: socket_t()
{
    raw_socket = -1;
    is_unix = false;
}

socket_t :: socket_t( int socket, bool is_unix_socket )
{
    raw_socket = socket;
    is_unix = is_unix_socket;
}

bool socket_t :: create()
{
    close();

    reset_errno();
    if( ( raw_socket = (int)::socket( SOCK_AF, SOCK_STREAM, 0 ) ) < 0 )
    {
        set_error( "call to 'socket' failed" );
        return false;
    }

    reset_errno();
    int optional = 1;
    if( setsockopt( raw_socket, SOL_SOCKET, SO_REUSEADDR, ( char * )&optional, sizeof( optional ) ) < 0 )
    {
        set_error( "call to 'setsockopt' with SO_REUSEADDR failed" );
        close();
        return false;
    }

    return true;
}

bool socket_t :: create_unix()
{
    close();

    reset_errno();
    if( ( raw_socket = (int)::socket( AF_UNIX, SOCK_STREAM, 0 ) ) < 0 )
    {
        set_error( "call to 'socket' (unix) failed" );
        return false;
    }

    is_unix = true;
    return true;
}

bool socket_t :: set_nodelay()
{
    if( raw_socket == -1 )
    {
        reset_errno();
        set_error( "set_nodelay failed: socket == -1" );
        return false;
    }

    if( is_unix == true ) return true;

    int flag = 1;
    if( setsockopt( raw_socket, IPPROTO_TCP, TCP_NODELAY, ( const char * )&flag, sizeof( int ) ) < 0 )
    {
        set_error( "call to 'setsockopt' with TCP_NODELAY failed." );
        close();
        return false;
    }

    return true;
}

bool socket_t :: connect( const char * ip, int port )
{
    if( raw_socket == -1 )
    {
        reset_errno();
        set_error( "connect failed: socket == -1" );
        return false;
    }

    if( is_unix == true )
    {
        reset_errno();
        set_error( "socket mismatch: inet 'connect' called for an unix socket" );
        return false;
    }

    struct sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons( port );
    dest_addr.sin_addr.s_addr = inet_addr( ip );
    memset( dest_addr.sin_zero, '\0', sizeof( dest_addr.sin_zero ) );

    reset_errno();
    if( ::connect( raw_socket, ( struct sockaddr * )&dest_addr, sizeof( dest_addr ) ) < 0 )
    {
        set_error( "unable to connect to %s on port %i", ip, port );
        return false;
    }
    return true;
}

bool socket_t :: connect_unix( const char * path )
{
    if( raw_socket == -1 )
    {
        reset_errno();
        set_error( "connect_unix failed: socket == -1" );
        return false;
    }

    if( is_unix == false )
    {
        reset_errno();
        set_error( "socket mismatch: unix 'connect' called for an inet socket" );
        return false;
    }

    struct sockaddr_un dest_addr;

    dest_addr.sun_family = AF_UNIX;
    strncpy( dest_addr.sun_path, path, sizeof( dest_addr.sun_path ) );

    reset_errno();
    if( ::connect( raw_socket, ( struct sockaddr * )&dest_addr, sizeof( dest_addr ) ) < 0 )
    {
        set_error( "unable to connect to unix socket in '%s'", path );
        return false;
    }
    return true;
}

bool socket_t :: bind( unsigned port )
{
    if( raw_socket == -1 )
    {
        reset_errno();
        set_error( "bind failed: socket == -1" );
        return false;
    }

    if( is_unix == true )
    {
        reset_errno();
        set_error( "socket mismatch: inet 'bind' called for an unix socket" );
        return false;
    }

    struct sockaddr_in sock_address;
    memset( &sock_address, 0, sizeof( struct sockaddr_in ) );

    sock_address.sin_family = SOCK_AF;
    sock_address.sin_port = htons( port );

    reset_errno();
    if( ::bind( raw_socket, ( const struct sockaddr * )&sock_address, sizeof( sock_address ) ) < 0 )
    {
        set_error( "bind failed" );
        return false;
    }

    reset_errno();
    if( listen( raw_socket, 3 ) < 0 )
    {
        set_error( "listen failed" );
        return false;
    }

    return true;
}

bool socket_t :: bind_unix( const char * path )
{
    if( raw_socket == -1 )
    {
        reset_errno();
        set_error( "bind_unix failed: socket == -1" );
        return false;
    }

    if( is_unix == false )
    {
        reset_errno();
        set_error( "socket mismatch: unix 'bind' called for an inet socket" );
        return false;
    }

    struct sockaddr_un sock_address;
    memset( &sock_address, 0, sizeof( struct sockaddr_un ) );

    sock_address.sun_family = AF_UNIX;
    strncpy( sock_address.sun_path, path, sizeof( sock_address.sun_path ) );
    unlink( path );

    if( ::bind( raw_socket, ( const struct sockaddr * )&sock_address, sizeof( sock_address ) ) < 0 )
    {
        set_error( "bind (unix) failed" );
        return false;
    }

    reset_errno();
    if( listen( raw_socket, 3 ) < 0 )
    {
        set_error( "listen (unix) failed" );
        return false;
    }

    return true;
}

socket_t socket_t :: accept()
{
    struct sockaddr_in sock_address;
    int client_socket = 0;
    socklen_t size = 0;

    reset_errno();
    size = sizeof( sock_address );
    if( ( client_socket = (int)::accept( raw_socket, ( struct sockaddr * )&sock_address, &size ) ) < 0 )
    {
        set_error( "accept failed" );
        return socket_t();
    }

    return socket_t( client_socket, is_unix );
}

bool socket_t :: getip( std :: string& ip )
{
    if( is_unix == true )
    {
        ip = "127.0.0.1";
        return true;
    }

    struct sockaddr_in sock_address;
    socklen_t length = sizeof( sock_address );

    reset_errno();
    if( getpeername( raw_socket, ( struct sockaddr * )&sock_address, &length ) < 0 )
    {
        set_error( "getpeername failed" );
        return false;
    }

    unsigned raw_ip = ntohl( sock_address.sin_addr.s_addr );
    std :: stringstream stream;

    stream << (( raw_ip >> 24 ) & 0xFF) << '.' << (( raw_ip >> 16 ) & 0xFF) << '.' << (( raw_ip >> 8 ) & 0xFF) << '.' << ( raw_ip & 0xFF );
    ip = stream.str();

    return true;
}

int socket_t :: write( const char * buffer, int length )
{
    return send( raw_socket, buffer, length, 0 );
}

bool socket_t :: full_read( char * buffer, int length )
{
    int start;
    int block;
    int rd;

    for( start = 0; start < length; start += rd )
    {
        block = std :: min( length - start, 2048 );
        rd = recv( raw_socket, buffer + start, block, 0 );

        if( rd <= 0 ) return false;
    }

    return true;
}


bool socket_t :: full_write( const char * buffer, int length )
{
    int start;
    int wrote;
    int block;

    for( start = 0; start < length; start += wrote )
    {
        block = std :: min( length - start, 2048 );
        wrote = send( raw_socket, buffer + start, block, 0 );

        if( wrote <= 0 ) return false;
    }

    return true;
}

int socket_t :: read( char * buffer, int length )
{
    return recv( raw_socket, buffer, length, 0 );
}

bool socket_t :: is_valid()
{
    return raw_socket >= 0;
}

void socket_t :: close()
{
    if( is_valid() == false ) return;
    #ifdef USE_NATIVE_WINDOWS_SOCKETS
        closesocket( raw_socket );
    #else
        ::close( raw_socket );
    #endif
    raw_socket = -1;
    is_unix = false;
}
