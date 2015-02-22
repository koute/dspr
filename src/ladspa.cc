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
#include <ladspa.h>

class dspr_filter_t : public dspr_client_t
{
    public:

        union
        {
            struct
            {
                LADSPA_Data * m_input_left;
                LADSPA_Data * m_output_left;
                LADSPA_Data * m_input_right;
                LADSPA_Data * m_output_right;
            };
            LADSPA_Data * m_ports[ 4 ];
        };
};

static const char * const c_port_names[ 4 ] =
{
    "Input",
    "Output",
    "Input",
    "Output"
};

static const LADSPA_PortDescriptor c_ports[ 4 ] =
{
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO,
    LADSPA_PORT_INPUT  | LADSPA_PORT_AUDIO,
    LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO
};

static const LADSPA_PortRangeHint c_port_range_hints[ 4 ] =
{
    {0},
    {0},
    {0},
    {0}
};

void create_descriptor( LADSPA_Descriptor * descriptor, int channels );

static LADSPA_Descriptor g_descriptor_ch1;
static LADSPA_Descriptor g_descriptor_ch2;

static bool g_descriptors_initialized = false;

extern "C"
{
    #pragma GCC visibility push( default )

    const LADSPA_Descriptor * ladspa_descriptor( unsigned long index )
    {
        if( g_descriptors_initialized == false )
        {
            create_descriptor( &g_descriptor_ch1, 1 );
            create_descriptor( &g_descriptor_ch2, 2 );
            g_descriptors_initialized = true;
        }

        switch( index )
        {
            case 0:
                return &g_descriptor_ch1;
            case 1:
                return &g_descriptor_ch2;
            default:
                return 0;
        }
    }

    LADSPA_Handle ladspa_instantiate( const LADSPA_Descriptor * descriptor, unsigned long sample_rate )
    {
        dspr_filter_t * filter = new dspr_filter_t;
        filter->initialize();
        filter->set_sample_rate( sample_rate );
        filter->set_channels( descriptor->PortCount / 2 );

        return ( LADSPA_Handle )filter;
    }

    void ladspa_activate( LADSPA_Handle instance )
    {
        dspr_filter_t * filter = ( dspr_filter_t * )instance;
        if( filter == 0 )
        {
            fprintf( stderr, "ladspa_activate: instance == 0\n" );
            return;
        }

        filter->deactivate();
        filter->activate();
    }

    void ladspa_connect_port( LADSPA_Handle instance, unsigned long port, LADSPA_Data * data_location )
    {
        dspr_filter_t * filter = ( dspr_filter_t * )instance;
        if( filter == 0 )
        {
            fprintf( stderr, "ladspa_connect_port: instance == 0\n" );
            return;
        }

        filter->m_ports[ port ] = data_location;
    }

    void ladspa_run( LADSPA_Handle instance, unsigned long samples )
    {
        dspr_filter_t * filter = ( dspr_filter_t * )instance;
        if( filter == 0 )
        {
            fprintf( stderr, "ladspa_run: instance == 0\n" );
            return;
        }
        if( filter->is_active() == false && filter->try_activate() == false ) goto do_copy;

        if( filter->prepare_noninterleaved_float( samples ) == false ) goto failed;
        if( filter->write_channel( filter->m_input_left ) == false ) goto failed;
        if( filter->channels() == 2 )
            if( filter->write_channel( filter->m_input_right ) == false ) goto failed;
        if( filter->read_channel( filter->m_output_left ) == false ) goto failed;
        if( filter->channels() == 2 )
            if( filter->read_channel( filter->m_output_right ) == false ) goto failed;

        return;

        failed:

        do_copy:
            for( unsigned loop = 0; loop < samples; ++loop )
            {
                filter->m_output_left[ loop ] = filter->m_input_left[ loop ];
                filter->m_output_right[ loop ] = filter->m_input_right[ loop ];
            }
    }

    void ladspa_deactivate( LADSPA_Handle instance )
    {
        dspr_filter_t * filter = ( dspr_filter_t * )instance;
        if( filter == 0 )
        {
            fprintf( stderr, "ladspa_deactivate: instance == 0\n" );
            return;
        }

        filter->deactivate();
    }

    void ladspa_cleanup( LADSPA_Handle instance )
    {
        dspr_filter_t * filter = ( dspr_filter_t * )instance;
        if( filter == 0 )
        {
            fprintf( stderr, "ladspa_cleanup: instance == 0\n" );
            return;
        }

        delete filter;
    }

    #pragma GCC visibility pop
}

void create_descriptor( LADSPA_Descriptor * descriptor, int channels )
{
    memset( (void *)descriptor, 0, sizeof( LADSPA_Descriptor ) );
    descriptor->UniqueID = 100;

    if( channels == 1 )
        descriptor->Label = "dspr1";
    else if( channels == 2 )
    {
        descriptor->Label = "dspr2";
        descriptor->UniqueID++;
    }

    descriptor->Name = "DSPR";
    descriptor->Maker = "Jan Bujak <j@exia.io>";
    descriptor->Copyright = "Copyright (C) 2010";
    descriptor->PortCount = channels * 2;
    descriptor->PortDescriptors = c_ports;
    descriptor->PortNames = c_port_names;
    descriptor->PortRangeHints = c_port_range_hints;

    descriptor->instantiate = ladspa_instantiate;
    descriptor->activate = ladspa_activate;
    descriptor->connect_port = ladspa_connect_port;
    descriptor->run = ladspa_run;
    descriptor->deactivate = ladspa_deactivate;
    descriptor->cleanup = ladspa_cleanup;
}