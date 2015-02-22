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

#ifndef MODULE_WINAMP_H
#define MODULE_WINAMP_H

#include "processor.h"

struct module_winamp_private_t;
class module_winamp_t : public module_t
{
    public:

        module_winamp_t();
        virtual ~module_winamp_t();

        virtual format_representation representation()
        {
            return format_short;
        }

        virtual format_endianness endianness()
        {
            return format_le;
        }

        virtual format_interleaveness interleaveness()
        {
            return format_interleaved;
        }

        virtual bool inplace()
        {
            return true;
        }

        virtual bool are_acceptable( unsigned channels, unsigned sample_rate, format_t format )
        {
            if( channels == 2 ) return true;
            else return false;
        }

        virtual bool is_ready()
        {
            if( is_loaded() == true ) return true;
            if( tried_to_load == true ) return false;

            load();
            return is_loaded();
        }

        virtual meta_module_t * get_meta_module();
        virtual bool set_string( const char * key, const char * value );
        virtual bool get_string( const char * key, std :: string& value );
        virtual bool set_long( const char * key, long value );
        virtual bool get_long( const char * key, long& value );
        virtual bool execute_field( const char * key );
        virtual void process( const char * input, char * output, unsigned length, unsigned samples, unsigned channels, unsigned sample_rate, format_t format );

    private:

        std :: string plugin_path;
        unsigned sample_rate_hack;
        bool tried_to_load;

        bool load();
        void destroy();
        bool is_loaded();

        module_winamp_private_t * self;
};

class meta_module_winamp_t : public meta_module_t
{
    virtual const char * name()
    {
        return "winamp";
    }

    virtual const char * pretty_name()
    {
        return "WinAMP DSP Plugin Loader";
    }

    virtual const char * version()
    {
        return "1.0";
    }

    virtual void construct_by_reference( module_t *& value )
    {
        value = new module_winamp_t();
    }

    virtual const char ** fields();
    virtual bool is_readonly( const char * key );
    virtual field_type get_type( const char * key );
};

extern meta_module_winamp_t g_meta_module_winamp;

#endif
