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

#ifndef DYNAMIC_BUFFER_H
#define DYNAMIC_BUFFER_H

#include "dspr.h"

class dynamic_buffer_t
{
    public:

    dynamic_buffer_t()
    {
        m_buffer = ( char * )malloc( 8 * 1024 );
        m_buffer_size = 8 * 1024;
        m_buffer_length = 0;
        m_buffer_position = 0;
    }

    ~dynamic_buffer_t()
    {
        free( m_buffer );
    }

    void resize( size_t size )
    {
        if( size == m_buffer_size ) return;
        m_buffer = ( char * )realloc( ( void * )m_buffer, size );

        if( size < m_buffer_size ) m_buffer_length = std :: min( m_buffer_length, m_buffer_size );
        m_buffer_size = size;
    }

    void resize_to_at_least( size_t size )
    {
        if( size <= m_buffer_size ) return;
        m_buffer = ( char * )realloc( ( void * )m_buffer, size );
        m_buffer_size = size;
    }

    void append( const char * data, size_t length )
    {
        if( m_buffer_length + length >= m_buffer_size ) resize( m_buffer_length + length * 2 );

        memcpy( ( void * )( m_buffer + m_buffer_length ), ( const void * )data, length );
        m_buffer_length += length;
    }

    void register_manual_append( size_t length )
    {
        m_buffer_length += length;
    }

    void move( int nth )
    {
        m_buffer_position = std :: min( size_t( std :: max( int( m_buffer_position ) + nth, 0 ) ), m_buffer_length );
    }

    void clear()
    {
        m_buffer_length = 0;
        m_buffer_position = 0;
    }

    void reset()
    {
        m_buffer_position = 0;
    }

    bool empty()
    {
        return m_buffer_length == 0;
    }

    unsigned read_unsigned()
    {
        unsigned result = *(( unsigned * )buffer());
        move( sizeof( unsigned ) );

        return result;
    }

    char * buffer_start()      { return m_buffer; }
    char * buffer()            { return m_buffer + m_buffer_position; }
    char * buffer_end()        { return m_buffer + m_buffer_length; }
    size_t size()              { return m_buffer_size; }
    size_t length()            { return m_buffer_length - m_buffer_position; }
    size_t absolute_length()   { return m_buffer_length; }

    private:

    dynamic_buffer_t( const dynamic_buffer_t& copy )
    {
    }

    char * m_buffer;
    size_t m_buffer_size;
    size_t m_buffer_length;
    size_t m_buffer_position;
};

#endif
