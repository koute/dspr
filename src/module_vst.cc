/*
    Copyright 2010 (C)] Jan Bujak <j@exia.io>

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

#include "module_vst.h"
/* #include "vst/aeffectx.h" */

/* Since the VST headers are non-free we redefine stuff we need ourselves. */
struct AEffect;

static const int32_t kEffectMagic = 0x56737450; // 'Vstp'

enum AudioMasterOpcodes
{
	audioMasterAutomate = 0,
	audioMasterVersion  = 1,
	audioMasterIdle     = 3
};

enum AEffectOpcodes
{
    effOpen            = 0,
    effClose           = 1,
    effGetParamDisplay = 7,
    effSetSampleRate   = 10,
    effSetBlockSize    = 11,
    effMainsChanged    = 12,
    effEditGetRect     = 13,
    effEditOpen        = 14,
    effEditIdle        = 19,

};

enum AEffectXOpcodes
{
    effGetEffectName    = 45,
    effGetVendorString  = 47,
    effGetProductString = 48
};

enum VstAEffectFlags
{
    effFlagsHasEditor = 1 << 0
};

typedef intptr_t ( *AEffectDispatcherProc )( AEffect * effect, int32_t opcode, int32_t index, intptr_t value, void * ptr, float opt );
typedef void ( *AEffectProcessProc )( AEffect * effect, float ** inputs, float ** outputs, int32_t sampleFrames );
typedef void ( *AEffectSetParameterProc )( AEffect * effect, int32_t index, float parameter );
typedef float ( *AEffectGetParameterProc )( AEffect * effect, int32_t index );
typedef	intptr_t ( *audioMasterCallback ) ( AEffect * effect, int32_t opcode, int32_t index, intptr_t value, void * ptr, float opt );

struct AEffect
{
	int32_t magic;

	AEffectDispatcherProc dispatcher;
	AEffectProcessProc process;
	AEffectSetParameterProc setParameter;
	AEffectGetParameterProc getParameter;

	int32_t numPrograms;
	int32_t numParams;
	int32_t numInputs;
	int32_t numOutputs;
	int32_t flags;
	intptr_t resvd1;
	intptr_t resvd2;
	int32_t initialDelay;
	int32_t realQualities;
	int32_t offQualities;
	float ioRatio;
	void * object;
	void * user;
	int32_t uniqueID;
	int32_t version;
	AEffectProcessProc processReplacing;

	char padding[ 60 ];
};

struct ERect
{
	int16_t top;
	int16_t left;
	int16_t bottom;
	int16_t right;
};

extern "C"
{
    intptr_t vst_host_callback( AEffect * effect, int32_t opcode, int32_t index, int32_t value, void * ptr, float opt )
    {
        switch( opcode )
        {
            case audioMasterAutomate:
                break;

            case audioMasterVersion:
                return 2400;

            case audioMasterIdle:
                effect->dispatcher( effect, effEditIdle, 0, 0, 0, 0 );
                break;

            case effGetParamDisplay:
                printf( "vst: effGetParamDisplay called: %i\n", value );
                break;

            default:
                printf( "vst: plugin requested a value of opcode %d\n", opcode );
                break;
        }

        return 0;
    }
}

typedef AEffect *( * vst_plugin_entry_t )( audioMasterCallback host );

struct module_vst_private_t
{
    HINSTANCE library;

    vst_plugin_entry_t vst_entry_point;
    AEffect * vst_plugin;
    AEffectDispatcherProc vst_dispatcher;
};

meta_module_vst_t g_meta_module_vst;

meta_module_t * module_vst_t :: get_meta_module()
{
    return &g_meta_module_vst;
}

const char ** meta_module_vst_t :: fields()
{
    static const char * array[] = { "plugin-path", "load", "editor", 0 };
    return array;
}

bool meta_module_vst_t :: is_readonly( const char * key )
{
    return false;
}

field_type meta_module_vst_t :: get_type( const char * key )
{
    if( !strcmp( key, "plugin-path" ) ) return field_string;
    if( !strcmp( key, "load" ) || !strcmp( key, "editor" ) ) return field_executable;
    return field_long;
}

bool module_vst_t :: set_string( const char * key, const char * value )
{
    if( !strcmp( key, "plugin-path" ) )
    {
        plugin_path = value;
        tried_to_load = false;
        return true;
    }

    return false;
}

bool module_vst_t :: get_string( const char * key, std :: string& value )
{
    if( !strcmp( key, "plugin-path" ) )
    {
        value = plugin_path;
        return true;
    }

    if( is_loaded() == false ) return false;
    if( !strcmp( key, "effect-name" ) )
    {
        char buffer[ 256 ] = { 0 };
        self->vst_dispatcher( self->vst_plugin, effGetEffectName, 0, 0, buffer, 0 );
        value = buffer;
        return true;
    }
    if( !strcmp( key, "vendor" ) )
    {
        char buffer[ 256 ] = { 0 };
        self->vst_dispatcher( self->vst_plugin, effGetVendorString, 0, 0, buffer, 0 );
        value = buffer;
        return true;
    }
    if( !strcmp( key, "product-name" ) )
    {
        char buffer[ 256 ] = { 0 };
        self->vst_dispatcher( self->vst_plugin, effGetProductString, 0, 0, buffer, 0 );
        value = buffer;
        return true;
    }

    return false;
}

bool module_vst_t :: set_long( const char * key, long value )
{
    return false;
}

bool module_vst_t :: get_long( const char * key, long& value )
{
    return false;
}

DWORD WINAPI plugin_editor_thread( LPVOID param );

bool module_vst_t :: execute_field( const char * key )
{
    if( !strcmp( key, "load" ) )
    {
        load();
        return true;
    }
    if( !strcmp( key, "editor" ) )
    {
        if( CreateThread( NULL, 0, plugin_editor_thread, ( LPVOID )this, 0, NULL ) == NULL )
        {
            reset_errno();
            set_error( "could not create a new thread for the editor" );
            return false;
        }
        return true;
    }
    return false;
}

module_vst_t :: module_vst_t()
{
    sample_rate = 44100;
    tried_to_load = false;
    self = new module_vst_private_t;

    self->library = 0;
    self->vst_entry_point = 0;
    self->vst_plugin = 0;
    self->vst_dispatcher = 0;
}

module_vst_t :: ~module_vst_t()
{
    destroy();
    delete self;
}

static LRESULT WINAPI wndproc( HWND window, UINT message, WPARAM wp, LPARAM lp )
{
    switch( message )
    {
        case WM_CLOSE:
        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;
    }

    return DefWindowProcA( window, message, wp, lp );
}

DWORD WINAPI plugin_editor_thread( LPVOID param )
{
    module_vst_t * module = ( module_vst_t * )param;
    module_vst_private_t * self = module->self;

    if( !(self->vst_plugin->flags & effFlagsHasEditor) )
    {
        reset_errno();
        set_error( "vst plugin doesn't support an editor" );
        return 0;
    }

    HMODULE hinstance;
    HWND window;

    if( (hinstance = GetModuleHandleA( NULL )) == 0 )
    {
        reset_errno();
        set_error( "unable to acquire hinstance" );
        return 0;
    }

    char class_name[ 1024 ];
    sprintf( class_name, "FST_%p", self );

    char menu_name[ 1024 ];
    sprintf( menu_name, "%s_MENU", class_name );

    WNDCLASSA wc;
    memset( &wc, 0, sizeof( WNDCLASSA ) );

    wc.lpfnWndProc = wndproc;
    wc.hInstance = hinstance;
    wc.hCursor = LoadCursorA( NULL, IDI_APPLICATION );
    wc.lpszMenuName = menu_name;
    wc.lpszClassName = class_name;

    if( !RegisterClassA( &wc ) )
    {
        reset_errno();
        set_error( "unable to register a new window class for the editor" );
        return 0;
    }

    window = CreateWindowExA( 0, class_name, "DSPR VST Host", ((WS_OVERLAPPEDWINDOW | WS_THICKFRAME) & ~WS_MAXIMIZEBOX), CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hinstance, NULL );

    if( window == 0 )
    {
        reset_errno();
        set_error( "unable to create a new window for the editor" );
        return 0;
    }

    struct ERect * er;

    self->vst_dispatcher( self->vst_plugin, effEditOpen, 0, 0, window, 0 );
    self->vst_dispatcher( self->vst_plugin, effEditGetRect, 0, 0, &er, 0 );

    unsigned width = er->right - er->left;
    unsigned height = er->bottom - er->top;

    printf( "vst editor dimensions: %dx%d\n", width, height );

    SetWindowPos( window, 0, 0, 0, width + 8, height + 26, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER );
    ShowWindow( window, SW_SHOWNORMAL );
    UpdateWindow( window );

    if( !SetTimer( window, 1000, 100, NULL ) )
    {
        reset_errno();
        set_error( "failed to set a timer on the editor's window" );
    }

    MSG message;

    while( GetMessageA( &message, NULL, 0, 0 ) )
    {
        TranslateMessage( &message );
        DispatchMessageA( &message );

        if( message.message == WM_TIMER )
            module->self->vst_dispatcher( module->self->vst_plugin, effEditIdle, 0, 0, NULL, 0 );
    }

    return 0;
}

bool module_vst_t :: load()
{
    destroy();

    if( ( self->library = LoadLibrary( plugin_path.c_str() ) ) == 0 )
    {
        reset_errno();
        set_error( "unable to load the dsp dll" );
        return false;
    }

    self->vst_entry_point = ( vst_plugin_entry_t  )GetProcAddress( self->library, "VSTPluginMain" );
    if( self->vst_entry_point == 0 )
        self->vst_entry_point = ( vst_plugin_entry_t  )GetProcAddress( self->library, "main" );
    if( self->vst_entry_point == 0 )
    {
        reset_errno();
        set_error( "unable to aquire neither the address of 'VSTPluginMain' nor 'main'" );
        destroy();
        return false;
    }

    self->vst_plugin = self->vst_entry_point( vst_host_callback );
    if( self->vst_plugin->magic != kEffectMagic )
    {
        reset_errno();
        set_error( "invalid VST magic number" );
        destroy();
        return false;
    }

    self->vst_dispatcher = ( AEffectDispatcherProc )( self->vst_plugin->dispatcher );

    self->vst_dispatcher( self->vst_plugin, effOpen, 0, 0, NULL, 0.0f );
    self->vst_dispatcher( self->vst_plugin, effSetSampleRate, 0, 0, NULL, float( sample_rate ) );
    self->vst_dispatcher( self->vst_plugin, effSetBlockSize, 0, 1024, NULL, 0.0f );

    self->vst_dispatcher( self->vst_plugin, effMainsChanged, 0, 1, NULL, 0.0f );

    printf( "vst: dsp plugin loaded\n" );

    return true;
}

void module_vst_t :: destroy()
{
    if( self->library )
    {
        if( self->vst_plugin )
            self->vst_dispatcher( self->vst_plugin, effClose, 0, 0, NULL, 0.0f );
        FreeLibrary( self->library );
        self->library = 0;
    }
}

bool module_vst_t :: is_loaded()
{
    return self->library != 0;
}

void module_vst_t :: process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format )
{
    float * inputs[ 16 ];
    float * outputs[ 16 ];

    for( unsigned i = 0; i < channels; ++i )
    {
        inputs[ i ] = ((float *)input) + i * samples;
        outputs[ i ] = ((float *)output) + i * samples;
    }

    if( this->sample_rate != sample_rate )
    {
        this->sample_rate = sample_rate;
        self->vst_dispatcher( self->vst_plugin, effSetSampleRate, 0, 0, NULL, float( sample_rate ) );
    }

    self->vst_plugin->processReplacing( self->vst_plugin, ( float ** )&inputs, ( float ** )&outputs, samples );
}
