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

#include "module_winamp.h"

struct dsp_module_t
{
    char *    description;
    HWND      parent;
    HINSTANCE dll_instance;

    void ( *config )         ( dsp_module_t * self );
    int  ( *initialize )     ( dsp_module_t * self );
    int  ( *modify_samples ) ( dsp_module_t * self, short int * samples, int samples_count, int bps, int channels, int sample_rate );
    void ( *quit )           ( dsp_module_t * self );

    void * user_data;
};

struct dsp_header_t
{
    int    version;
    char * description;

    dsp_module_t * ( *get_module )( int );
};

typedef dsp_header_t * ( *dsp_getheader_t )();

struct module_winamp_private_t
{
    HINSTANCE library;
    dsp_module_t * module;

    dsp_getheader_t header_function;
    dsp_header_t * header;
};

meta_module_winamp_t g_meta_module_winamp;

meta_module_t * module_winamp_t :: get_meta_module()
{
    return &g_meta_module_winamp;
}

const char ** meta_module_winamp_t :: fields()
{
    static const char * array[] = { "plugin-path", "samplerate-hack", "plugin-description", "load", "configure", 0 };
    return array;
}

bool meta_module_winamp_t :: is_readonly( const char * key )
{
    if( !strcmp( "plugin-description", key ) ) return true;
    return false;
}

field_type meta_module_winamp_t :: get_type( const char * key )
{
    if( !strcmp( key, "plugin-path" ) ) return field_string;
    if( !strcmp( key, "samplerate-hack" ) ) return field_long;
    if( !strcmp( key, "load" ) || !strcmp( key, "configure" ) ) return field_executable;
    return field_long;
}

bool module_winamp_t :: set_string( const char * key, const char * value )
{
    if( !strcmp( key, "plugin-path" ) )
    {
        plugin_path = value;
        tried_to_load = false;
        return true;
    }
    return false;
}

bool module_winamp_t :: get_string( const char * key, std :: string& value )
{
    if( !strcmp( key, "plugin-path" ) )
    {
        value = plugin_path;

        return true;
    }
    if( !strcmp( key, "plugin-description" ) )
    {
        if( is_loaded() == false ) return false;
        value = self->module->description;
        return true;
    }
    return false;
}

bool module_winamp_t :: set_long( const char * key, long value )
{
    if( !strcmp( key, "samplerate-hack" ) )
    {
        sample_rate_hack = value;
        return true;
    }
    return false;
}

bool module_winamp_t :: get_long( const char * key, long& value )
{
    if( !strcmp( key, "samplerate-hack" ) )
    {
        value = sample_rate_hack;
        return true;
    }
    return false;
}

bool module_winamp_t :: execute_field( const char * key )
{
    if( !strcmp( key, "load" ) )
    {
        load();
        return true;
    }
    if( !strcmp( key, "configure" ) )
    {
        if( is_loaded() == false ) return false;
        self->module->config( self->module );
        return true;
    }
    return false;
}

module_winamp_t :: module_winamp_t()
{
    sample_rate_hack = 0;
    tried_to_load = false;
    self = new module_winamp_private_t;

    self->library = 0;
    self->module = 0;

    self->header_function = 0;
    self->header = 0;
}

module_winamp_t :: ~module_winamp_t()
{
    destroy();
    delete self;
}

bool module_winamp_t :: load()
{
    destroy();

    if( ( self->library = LoadLibrary( plugin_path.c_str() ) ) == 0 )
    {
        reset_errno();
        set_error( "unable to load the dsp dll" );
        return false;
    }

    if( ( self->header_function = ( dsp_getheader_t )GetProcAddress( self->library, "winampDSPGetHeader2" ) ) == 0 )
    {
        reset_errno();
        set_error( "unable to aquire address of winampDSPGetHeader2" );
        destroy();
        return false;
    }

    if( ( self->header = self->header_function() ) == 0 )
    {
        reset_errno();
        set_error( "plugin returned null header" );
        destroy();
        return false;
    }

    if( ( self->module = self->header->get_module( 0 ) ) == 0 )
    {
        reset_errno();
        set_error( "unable to aquire DSP module" );
        destroy();
        return false;
    }

    self->module->parent = 0;
    self->module->dll_instance = self->library;

    if( self->module->initialize( self->module ) != 0 )
    {
        reset_errno();
        set_error( "unable to initialize module" );
        destroy();
        return false;
    }

    printf( "winamp: dsp plugin loaded\n" );

    /*
        OSS/3D has a nasty audio crappifing bug - to work around it one needs to
        press some element of it's interface _after_ it recieved a few samples.
    */
    if( self->module->description && !strcmp( self->module->description, "4Front OSS/3D R7 (OSPI)" ) )
    {
        printf( "winamp: oss/3d hack initialized\n" );

        if( sample_rate_hack == 0 ) sample_rate_hack = 44100;
        self->module->config( self->module );

        HWND window = 0;
        while( window == 0 )
            window = FindWindow( 0, "4Front OSS/3D R7" );

        Sleep( 1000 );

        RECT rect;
        GetWindowRect( window, &rect );

        int screen_width  = GetSystemMetrics( SM_CXSCREEN );
        int screen_height = GetSystemMetrics( SM_CYSCREEN );

        {
            short buffer[ 2 ];
            self->module->modify_samples( self->module, ( short * )&buffer, 1, sizeof( short ) * 8, 2, sample_rate_hack );
        }

        INPUT input[ 9 ];
        memset( (void *)&input, 0, sizeof( INPUT ) * 9 );
        for( unsigned loop = 0; loop < 9; ++loop )
            input[loop].type = INPUT_MOUSE;

        input[0].type = INPUT_MOUSE;
        input[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
        input[0].mi.dx = DWORD( double(rect.left + 395) * (65535.0/double(screen_width)));
        input[0].mi.dy = DWORD( double(rect.bottom - 170) * (65535.0/double(screen_height)));

        input[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        input[3].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[4].mi.dwFlags = MOUSEEVENTF_LEFTUP;
        input[5].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        input[6].mi.dwFlags = MOUSEEVENTF_LEFTUP;

        SendInput( 7, &input[0], sizeof( INPUT ) );

        /* CloseWindow( window ); */
    }

    return true;
}

void module_winamp_t :: destroy()
{
    if( self->module )
    {
        self->module->quit( self->module );
        self->module = 0;
    }
    if( self->library )
    {
        FreeLibrary( self->library );
        self->library = 0;
    }
}

bool module_winamp_t :: is_loaded()
{
    return self->module != 0;
}

void module_winamp_t :: process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format )
{
    self->module->modify_samples( self->module, (short *)input, samples, sizeof( short ) * 8, channels, (sample_rate_hack > 0) ? (sample_rate_hack) : (sample_rate) );
}
