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

#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "dspr.h"

#if defined( i386 ) || defined( _X86_ ) || defined( __i386__ )
    #ifndef __LITTLE_ENDIAN__
        #define __LITTLE_ENDIAN__
    #endif
#endif

#if defined( __LITTLE_ENDIAN__ ) || (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define IS_LITTLE_ENDIAN true
    #define IS_BIG_ENDIAN false
    #define format_native_endianness format_le
#elif defined( __BIG_ENDIAN__ ) || (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    #define IS_LITTLE_ENDIAN false
    #define IS_BIG_ENDIAN true
    #define format_native_endianness format_be
#else
    #error "Unknown endianess!"
#endif

enum format_representation
{
    format_short,
    format_float,
    format_any_representation
};

enum format_endianness
{
    format_le,
    format_be,
    format_endianless
};

enum format_interleaveness
{
    format_interleaved,
    format_noninterleaved,
    format_any_interleaveness
};

struct format_t
{
    format_t()
    {
    }

    format_t( format_representation representation, format_endianness endianness, format_interleaveness interleaveness )
    {
        this->representation = representation;
        this->endianness     = endianness;
        this->interleaveness = interleaveness;
    }

    format_representation representation;
    format_endianness     endianness;
    format_interleaveness interleaveness;
};

class meta_module_t;
class module_t;
class processor_t
{
    public:

        processor_t();
        ~processor_t();

        void process( const char * buffer, size_t length, format_representation representation, format_endianness endianness, format_interleaveness interleaveness, unsigned channels, unsigned sample_rate );

        const char * buffer();
        unsigned buffer_length();

        /* Input variables. */
        bool enabled;

        /* Output variables. */
        bool pending;
        bool skipped;

        std :: list< module_t * > modules;

        module_t * construct_module( const char * name );
        bool add_module( module_t * module );
        bool add_module( const char * name );

        static std :: vector< meta_module_t * > registered_modules;

        static bool register_module( meta_module_t * );
        static bool register_from_so( const char * filename );

    private:

        void convert( format_representation representation, format_endianness endianness, format_interleaveness interleaveness );
        void prepare_buffer( unsigned id, unsigned length );

        char * next_buffer();
        unsigned next_buffer_id();

        unsigned channels;
        unsigned current_buffer;

        struct
        {
            char * buffer;
            size_t length;
            size_t allocated;
            format_t format;
        } buffers[ 3 ];
};

enum field_type
{
    field_null,
    field_string,
    field_long,
    field_double,
    field_bool,
    field_executable
};

class meta_module_t
{
    public:

        virtual const char * name() = 0;
        virtual const char * pretty_name() = 0;
        virtual const char * version() = 0;

        virtual const char ** fields() = 0;

        virtual bool is_readonly( const char * key )
        {
            return false;
        }

        virtual field_type get_type( const char * key )
        {
            return field_null;
        }

        /* BUG: Due to a nasty bug in MinGW-4.2.1
                where construct() always returned 0
                we're forced to construct by reference.
        */
        virtual void construct_by_reference( module_t *& value ) = 0;

        bool has_field( const char * key )
        {
            const char ** array = fields();
            for( unsigned i = 0; array[ i ] != 0; ++i )
                if( !strcmp( array[ i ], key ) ) return true;

            return false;
        }

        bool is_executable( const char * key )
        {
            return get_type( key ) == field_executable;
        }

        module_t * construct()
        {
            module_t * module;
            construct_by_reference( module );

            return module;
        }
};

class module_t
{
    public:

        virtual ~module_t() { }

        virtual bool initialize()
        {
            return true;
        }

        virtual bool is_initialized()
        {
            return true;
        }

        virtual format_representation representation()
        {
            return format_any_representation;
        }

        virtual format_endianness endianness()
        {
            return format_endianless;
        }

        virtual format_interleaveness interleaveness()
        {
            return format_any_interleaveness;
        }

        virtual bool inplace()
        {
            return false;
        }

        virtual bool are_acceptable( unsigned channels, unsigned sample_rate, format_t format )
        {
            return true;
        }

        virtual bool is_ready()
        {
            return true;
        }

        virtual bool set_string( const char * key, const char * value )
        {
            return false;
        }

        virtual bool get_string( const char * key, std :: string& value )
        {
            return false;
        }

        virtual bool set_double( const char * key, double value )
        {
            return false;
        }

        virtual bool get_double( const char * key, double& value )
        {
            return false;
        }

        virtual bool set_long( const char * key, long value )
        {
            return false;
        }

        virtual bool get_long( const char * key, long& value )
        {
            return false;
        }

        virtual bool set_bool( const char * key, bool value )
        {
            return false;
        }

        virtual bool get_bool( const char * key, bool& value )
        {
            return false;
        }

        virtual bool execute_field( const char * key )
        {
            return false;
        }

        bool set_field( const char * key, const char * value )
        {
            if( set_string( key, value ) ) return true;
            if( set_long( key, atol( value ) ) ) return true;
            if( set_double( key, atof( value ) ) ) return true;
            if( !strcmp( value, "true" ) || !strcmp( value, "yes" ) || !strcmp( value, "y" ) || !strcmp( value, "1" ) )
            {
                if( set_bool( value, true ) ) return true;
            }
            else
            {
                if( set_bool( value, false ) ) return true;
            }

            return false;
        }

        virtual meta_module_t * get_meta_module() = 0;
        virtual void process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format ) = 0;
};

#endif
