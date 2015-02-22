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

#include "processor.h"

inline short int float2short( float sample )
{
    if( sample >= 1.0f ) return 32767;
    else return short( sample * 32767.0f );
}

inline float short2float( short int sample )
{
    if( sample == -32768 ) return -1.0;
    else return float( sample ) / float( 32767 );
}

template < bool from_interleaved, bool to_interleaved, bool swap_endian, typename input_t, typename output_t >
void convert_template( char * typeless_input, unsigned length, char ** typeless_output, size_t& output_length, size_t& output_allocated, unsigned channels )
{
    input_t * input = ( input_t * )typeless_input;
    output_t ** output = ( output_t ** )typeless_output;

    unsigned samples = length / (channels * sizeof( input_t ));
    output_length = samples * channels * sizeof( output_t );

    if( output_allocated < output_length )
    {
        *output = ( output_t * )realloc( ( void * )*output, output_length );
        output_allocated = output_length;
    }

    unsigned current_channel = 0;
    unsigned phase = 0;
    for( unsigned i = 0; i < samples * channels; ++i )
    {
        unsigned input_index;
        unsigned output_index;

        if( to_interleaved == from_interleaved )
        {
            input_index = i;
            output_index = i;
        }
        else
        {
            unsigned interleaved_index = current_channel * samples + phase;
            current_channel++;
            if( current_channel == channels )
            {
                current_channel = 0;
                phase += 1;
            }

            if( from_interleaved == true && to_interleaved == false )
            {
                input_index = i;
                output_index = interleaved_index;
            }
            else if( from_interleaved == false && to_interleaved == true )
            {
                input_index = interleaved_index;
                output_index = i;
            }
        }

        input_t input_sample = input[ input_index ];
        output_t output_sample;
        if( sizeof( input_sample ) == sizeof( short ) && sizeof( output_sample ) == sizeof( float ) )
            output_sample = short2float( input_sample );
        else if( sizeof( input_sample ) == sizeof( float ) && sizeof( output_sample ) == sizeof( short ) )
            output_sample = float2short( input_sample );
        else
            output_sample = input_sample;

        if( swap_endian == true )
        {
            if( sizeof( output_sample ) == sizeof( short ) )
                output_sample = (short(output_sample) << 8) | (short(output_sample) >> 8);
            else if( sizeof( output_sample ) == sizeof( float ) )
            {
                union
                {
                    float floating_point;
                    unsigned numeric;
                } u;
                u.floating_point = output_sample;

                u.numeric = (u.numeric << 24) |
                            ( (u.numeric <<  8) & 0x00ff0000 ) |
                            ( (u.numeric >>  8) & 0x0000ff00 ) |
                            (  u.numeric >> 24);

                output_sample = u.floating_point;

            }
        }

        (*output)[ output_index ] = output_sample;
    }
}

processor_t :: processor_t()
{
    enabled = true;
    pending = false;
    skipped = false;

    current_buffer = 0;

    memset( (void *)&buffers[ 0 ], 0, sizeof( buffers ) );
}

processor_t :: ~processor_t()
{
    for( std :: list< module_t * > :: iterator i = modules.begin(); i != modules.end(); ++i )
    {
        delete (*i);
    }
}

void processor_t :: process( const char * input, size_t length, format_representation representation, format_endianness endianness, format_interleaveness interleaveness, unsigned channels, unsigned sample_rate )
{
    if( enabled == false )
    {
        skipped = true;
        return;
    }

    if( endianness == format_endianless )
        endianness = format_native_endianness;

    pending = true;

    buffers[ 0 ].buffer = ( char * )input;
    buffers[ 0 ].length = length;
    buffers[ 0 ].format = format_t( representation, endianness, interleaveness );
    current_buffer = 0;

    if( modules.empty() ) return;
    this->channels = channels;

    for( std :: list< module_t * > :: iterator i = modules.begin(); i != modules.end(); ++i )
    {
        module_t * module = *i;

        if( module->is_initialized() == false ) continue;

        format_representation module_representation = module->representation();
        format_endianness module_endianness = module->endianness();
        format_interleaveness module_interleaveness = module->interleaveness();

        if( module_representation == format_any_representation )
            module_representation = representation;

        if( module_endianness == format_endianless )
            module_endianness = endianness;

        if( module_interleaveness == format_any_interleaveness )
            module_interleaveness = interleaveness;

        format_t format( module_representation, module_endianness, module_interleaveness );
        if( module->are_acceptable( channels, sample_rate, format ) == false ) continue;
        if( module->is_ready() == false ) continue;

        convert( module_representation, module_endianness, module_interleaveness );
        unsigned samples = buffer_length() / (channels * (module_representation == format_float ? sizeof( float ) : sizeof( short )) );

        /// printf( "processing... %i (%i) %i\n", buffer_length(), current_buffer, samples );
        /// printf( "0 - %p\n1 - %p, 2 - %p\n", buffers[ 0 ].buffer, buffers[ 1 ].buffer, buffers[ 2 ].buffer );
        if( module->inplace() == true )
        {
            module->process( buffer(), ( char * )buffer(), buffer_length(), samples, channels, sample_rate, format );
        }
        else
        {
            prepare_buffer( next_buffer_id(), buffers[ current_buffer ].length );
            module->process( buffer(), next_buffer(), buffer_length(), samples, channels, sample_rate, format );
        }
    }

/// printf( "reconverting\n");
    convert( representation, endianness, interleaveness );
    /// printf( "conversion done; was %i, is %i (%i)\n", buffers[ 0 ].length, buffer_length(), current_buffer );
}

const char * processor_t :: buffer()
{
    return buffers[ current_buffer ].buffer;
}

unsigned processor_t :: buffer_length()
{
    return buffers[ current_buffer ].length;
}

char * processor_t :: next_buffer()
{
    return buffers[ next_buffer_id() ].buffer;
}

void processor_t :: prepare_buffer( unsigned id, unsigned length )
{
    if( buffers[ id ].allocated < length )
    {
        buffers[ id ].allocated = length;
        buffers[ id ].buffer = (char *)realloc( (void *)buffers[ id ].buffer, buffers[ id ].allocated );
    }
}

void processor_t :: convert( format_representation output_representation, format_endianness output_endianness, format_interleaveness output_interleaveness )
{
    char * input = buffers[ current_buffer ].buffer;
    char * output = buffers[ next_buffer_id() ].buffer;
    size_t& output_length = buffers[ next_buffer_id() ].length;
    size_t& output_allocated = buffers[ next_buffer_id() ].allocated;

    format_representation input_representation = buffers[ current_buffer ].format.representation;
    format_endianness input_endianness = buffers[ current_buffer ].format.endianness;
    format_interleaveness input_interleaveness = buffers[ current_buffer ].format.interleaveness;

    /// printf( "%i %i %i -> %i %i %i\n", input_representation, input_endianness, input_interleaveness, output_representation, output_endianness, output_interleaveness );

    if( input_representation == output_representation &&
        input_endianness == output_endianness &&
        input_interleaveness == output_interleaveness /* &&
        current_buffer != 0 */ ) return;

    #include "processor-conversion-ifs.cc"

    buffers[ next_buffer_id() ].buffer = output;
    buffers[ next_buffer_id() ].format = format_t( output_representation, output_endianness, output_interleaveness );
    current_buffer = next_buffer_id();
}

unsigned processor_t :: next_buffer_id()
{
    if( current_buffer == 2 ) return 1;
    else return current_buffer + 1;
}

bool processor_t :: add_module( module_t * module )
{
    if( module == 0 ) return false;
    modules.push_back( module );

    return true;
}

bool processor_t :: add_module( const char * name )
{
    return add_module( construct_module( name ) );
}

module_t * processor_t :: construct_module( const char * name )
{
    for( std :: vector< meta_module_t * > :: iterator i = registered_modules.begin(); i != registered_modules.end(); ++i )
    {
        if( strcmp( (*i)->name(), name ) ) continue;
        module_t * module;
        (*i)->construct_by_reference( module );

        if( module == 0 )
            fprintf( stderr, "warning: .construct() for module '%s' returned nil\n", name );
        return module;
    }

    return 0;
}

std :: vector< meta_module_t * > processor_t :: registered_modules;

bool processor_t :: register_module( meta_module_t * module )
{
    registered_modules.push_back( module );
    return true;
}

typedef meta_module_t * (*get_meta_module_t)();

bool processor_t :: register_from_so( const char * filename )
{
    #ifndef WINDOWS
        void * handle = dlopen( filename, RTLD_LAZY | RTLD_GLOBAL );
        if( handle == 0 )
        {
            set_error( "unable to open so" );
            return false;
        }

        get_meta_module_t get_meta_module = ( get_meta_module_t )dlsym( handle, "get_dspr_meta_module" );
        if( get_meta_module == 0 )
        {
            set_error( "'get_dspr_meta_module' not found in so" );
            return false;
        }
    #else
        HINSTANCE handle = LoadLibraryA( filename );
        if( handle == 0 ) return false;

        get_meta_module_t get_meta_module = ( get_meta_module_t )GetProcAddress( handle, "get_dspr_meta_module" );
        if( get_meta_module == 0 ) return false;
    #endif

    return register_module( get_meta_module() );
}
